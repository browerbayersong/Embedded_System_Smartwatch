package com.example.smartwatch

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Handler
import android.os.Looper
import java.io.InputStream
import java.io.OutputStream
import java.util.UUID

class BleManager(private val context: Context) {
    private val bluetoothManager = context.getSystemService(BluetoothManager::class.java)
    private val bluetoothAdapter: BluetoothAdapter? = bluetoothManager?.adapter

    // SPP (Serial Port Profile) UUID — 经典蓝牙串口协议
    private val sppUuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    private val mainHandler = Handler(Looper.getMainLooper())
    private var discoveryReceiver: BroadcastReceiver? = null
    private var connectThread: ConnectThread? = null
    private var connectedThread: ConnectedThread? = null

    fun isSupported(): Boolean = bluetoothAdapter != null

    fun isBluetoothEnabled(): Boolean = bluetoothAdapter?.isEnabled == true

    // ================ 经典蓝牙设备扫描 ================
    @SuppressLint("MissingPermission")
    fun startScan(
        onDeviceFound: (BluetoothDevice) -> Unit,
        onScanStatus: (String) -> Unit
    ) {
        val adapter = bluetoothAdapter
        if (adapter == null) {
            onScanStatus("设备不支持蓝牙")
            return
        }
        if (!adapter.isEnabled) {
            onScanStatus("请先打开蓝牙")
            return
        }

        // 如果之前有扫描，先停掉
        stopScan()

        // 注册广播接收器接收发现结果
        val receiver = object : BroadcastReceiver() {
            override fun onReceive(context: Context, intent: Intent) {
                when (intent.action) {
                    BluetoothDevice.ACTION_FOUND -> {
                        val device = intent.getParcelableExtra<BluetoothDevice>(BluetoothDevice.EXTRA_DEVICE)
                        if (device != null) {
                            // 过滤掉没有名字的设备和信号太弱的
                            val name = device.name
                            if (!name.isNullOrBlank()) {
                                onDeviceFound(device)
                            }
                        }
                    }
                    BluetoothAdapter.ACTION_DISCOVERY_FINISHED -> {
                        onScanStatus("扫描完成")
                    }
                }
            }
        }
        discoveryReceiver = receiver

        val filter = IntentFilter().apply {
            addAction(BluetoothDevice.ACTION_FOUND)
            addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED)
        }
        context.registerReceiver(receiver, filter)

        try {
            adapter.startDiscovery()
            onScanStatus("正在扫描经典蓝牙设备...")
        } catch (exc: Exception) {
            onScanStatus("扫描异常: ${exc.message}")
        }
    }

    @SuppressLint("MissingPermission")
    fun stopScan() {
        try {
            bluetoothAdapter?.cancelDiscovery()
        } catch (_: Exception) {}
        discoveryReceiver?.let {
            try {
                context.unregisterReceiver(it)
            } catch (_: Exception) {}
            discoveryReceiver = null
        }
    }

    // ================ 经典蓝牙 SPP 连接 ================
    @SuppressLint("MissingPermission")
    fun connect(
        device: BluetoothDevice,
        onConnectionStatus: (String) -> Unit,
        onRawData: (ByteArray) -> Unit
    ) {
        disconnect()

        connectThread = ConnectThread(device, onConnectionStatus, onRawData)
        connectThread?.start()
    }

    @SuppressLint("MissingPermission")
    fun disconnect() {
        try {
            connectThread?.cancel()
        } catch (_: Exception) {}
        connectThread = null
        try {
            connectedThread?.cancel()
        } catch (_: Exception) {}
        connectedThread = null
    }

    // ================ 发送数据 ================
    fun sendTimeSync(onStatus: (String) -> Unit) {
        val thread = connectedThread
        if (thread == null) {
            onStatus("当前没有连接")
            return
        }
        val frame = BtProtocol.buildTimeSyncFrame()
        thread.write(frame, onStatus)
    }

    // ================ 内部连接线程 ================
    @SuppressLint("MissingPermission")
    private inner class ConnectThread(
        private val device: BluetoothDevice,
        private val onStatus: (String) -> Unit,
        private val onData: (ByteArray) -> Unit
    ) : Thread() {
        private var socket: BluetoothSocket? = null

        override fun run() {
            try {
                // 1. 创建 RFCOMM socket
                socket = device.createRfcommSocketToServiceRecord(sppUuid)
            } catch (e: Exception) {
                mainHandler.post { onStatus("创建 Socket 失败: ${e.message}") }
                return
            }

            try {
                bluetoothAdapter?.cancelDiscovery()
                mainHandler.post { onStatus("正在连接 ${device.name ?: device.address} ...") }

                // 2. 发起连接（阻塞）
                socket?.connect()
            } catch (e: Exception) {
                mainHandler.post { onStatus("连接失败: ${e.message}") }
                try {
                    socket?.close()
                } catch (_: Exception) {}
                socket = null
                return
            }

            val s = socket
            if (s != null) {
                connectedThread = ConnectedThread(s, onStatus, onData)
                connectedThread?.start()
                mainHandler.post { onStatus("已连接到 ${device.name ?: device.address}") }
            }
        }

        fun cancel() {
            try {
                socket?.close()
            } catch (_: Exception) {}
        }
    }

    // ================ 已连接后的数据收发线程 ================
    private inner class ConnectedThread(
        private val socket: BluetoothSocket,
        private val onStatus: (String) -> Unit,
        private val onData: (ByteArray) -> Unit
    ) : Thread() {
        private val inStream: InputStream
        private val outStream: OutputStream

        init {
            inStream = socket.inputStream
            outStream = socket.outputStream
        }

        // 持续读取数据流，按帧解析
        override fun run() {
            val buffer = ByteArray(512)
            var bufferPos = 0

            while (true) {
                try {
                    val bytes = inStream.read(buffer, bufferPos, buffer.size - bufferPos)
                    if (bytes == -1) {
                        mainHandler.post { onStatus("对方已断开连接") }
                        break
                    }
                    bufferPos += bytes

                    // 尝试从 buffer 中解析出完整帧
                    var consumed = 0
                    while (true) {
                        // 查找帧头 0xAA
                        var frameStart = -1
                        for (i in consumed until bufferPos) {
                            if ((buffer[i].toInt() and 0xFF) == 0xAA) {
                                frameStart = i
                                break
                            }
                        }
                        if (frameStart == -1) {
                            // 没找到帧头，全部丢弃
                            consumed = bufferPos
                            break
                        }
                        // 帧头之后至少需要 3 字节（命令 + 长度 + 数据至少 0 字节 + 校验 + 帧尾）
                        if (bufferPos - frameStart < 5) {
                            break  // 数据不够，等下一批
                        }
                        val len = buffer[frameStart + 2].toInt() and 0xFF
                        val frameTotal = 5 + len
                        if (bufferPos - frameStart < frameTotal) {
                            break  // 完整帧还没收齐
                        }
                        // 检查帧尾
                        if ((buffer[frameStart + frameTotal - 1].toInt() and 0xFF) == 0x55) {
                            // 提取完整帧
                            val frame = buffer.sliceArray(frameStart until frameStart + frameTotal)
                            mainHandler.post { onData(frame) }
                            consumed = frameStart + frameTotal
                        } else {
                            // 帧尾不对，跳过这个帧头，继续找下一个
                            consumed = frameStart + 1
                        }
                    }

                    // 把剩余未处理的数据挪到 buffer 开头
                    if (consumed > 0) {
                        val remain = bufferPos - consumed
                        if (remain > 0) {
                            System.arraycopy(buffer, consumed, buffer, 0, remain)
                        }
                        bufferPos = remain
                    }
                } catch (e: Exception) {
                    mainHandler.post { onStatus("接收异常: ${e.message}") }
                    break
                }
            }
        }

        fun write(bytes: ByteArray, onStatus: (String) -> Unit) {
            try {
                outStream.write(bytes)
                outStream.flush()
                mainHandler.post { onStatus("已发送 ${bytes.size} 字节") }
            } catch (e: Exception) {
                mainHandler.post { onStatus("发送失败: ${e.message}") }
            }
        }

        fun cancel() {
            try {
                socket.close()
            } catch (_: Exception) {}
        }
    }
}