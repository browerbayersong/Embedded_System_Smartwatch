package com.example.smartwatch

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothGattService
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.Handler
import android.os.Looper
import java.util.UUID

class BleManager(private val context: Context) {
    private val bluetoothManager = context.getSystemService(BluetoothManager::class.java)
    private val bluetoothAdapter: BluetoothAdapter? = bluetoothManager?.adapter
    private val targetServiceUuid = UUID.fromString("0000ffe0-0000-1000-8000-00805f9b34fb")
    private val targetCharacteristicUuid = UUID.fromString("0000ffe1-0000-1000-8000-00805f9b34fb")
    private val descriptorUuid = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")

    private var bluetoothGatt: BluetoothGatt? = null
    private var notifyCharacteristic: BluetoothGattCharacteristic? = null
    private var scanCallback: ScanCallback? = null
    private val mainHandler = Handler(Looper.getMainLooper())

    fun isSupported(): Boolean = bluetoothAdapter != null

    fun isBluetoothEnabled(): Boolean = bluetoothAdapter?.isEnabled == true

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

        val scanner = adapter.bluetoothLeScanner
        if (scanner == null) {
            onScanStatus("无法获取 BLE 扫描器")
            return
        }

        stopScan()

        scanCallback = object : ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult) {
                val device = result.device
                val name = device.name ?: result.scanRecord?.deviceName
                if (!name.isNullOrBlank()) {
                    onDeviceFound(device)
                }
            }

            override fun onScanFailed(errorCode: Int) {
                onScanStatus("扫描失败: $errorCode")
            }
        }

        try {
            scanner.startScan(null, ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build(), scanCallback)
            onScanStatus("正在扫描 BLE 设备...")
        } catch (exc: Exception) {
            onScanStatus("扫描异常: ${exc.message}")
        }
    }

    @SuppressLint("MissingPermission")
    fun stopScan() {
        scanCallback?.let {
            bluetoothAdapter?.bluetoothLeScanner?.stopScan(it)
            scanCallback = null
        }
    }

    @SuppressLint("MissingPermission")
    fun connect(
        device: BluetoothDevice,
        onConnectionStatus: (String) -> Unit,
        onRawData: (ByteArray) -> Unit
    ) {
        disconnect()

        bluetoothGatt = device.connectGatt(context, false, object : BluetoothGattCallback() {
            override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
                if (newState == BluetoothGatt.STATE_CONNECTED) {
                    onConnectionStatus("已连接，正在发现服务")
                    gatt.discoverServices()
                } else if (newState == BluetoothGatt.STATE_DISCONNECTED) {
                    notifyCharacteristic = null
                    onConnectionStatus("已断开")
                }
            }

            override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
                if (status != BluetoothGatt.GATT_SUCCESS) {
                    onConnectionStatus("服务发现失败: $status")
                    return
                }

                val service: BluetoothGattService? = gatt.getService(targetServiceUuid)
                notifyCharacteristic = service?.getCharacteristic(targetCharacteristicUuid)
                if (notifyCharacteristic == null) {
                    onConnectionStatus("未找到特征 0000ffe1")
                    return
                }

                val enabled = gatt.setCharacteristicNotification(notifyCharacteristic, true)
                val descriptor = notifyCharacteristic?.getDescriptor(descriptorUuid)
                if (descriptor != null) {
                    descriptor.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
                    gatt.writeDescriptor(descriptor)
                }

                onConnectionStatus(if (enabled) "已连接，通知已开启" else "已连接，但无法开启通知")
            }

            override fun onCharacteristicChanged(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic) {
                if (characteristic.uuid == targetCharacteristicUuid) {
                    val bytes = characteristic.value ?: return
                    onRawData(bytes)
                }
            }

            override fun onCharacteristicWrite(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, status: Int) {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    mainHandler.post { onConnectionStatus("发送成功") }
                } else {
                    mainHandler.post { onConnectionStatus("写入失败: $status") }
                }
            }
        })
    }

    @SuppressLint("MissingPermission")
    fun disconnect() {
        bluetoothGatt?.close()
        bluetoothGatt = null
        notifyCharacteristic = null
    }

    @SuppressLint("MissingPermission")
    fun sendTimeSync(onStatus: (String) -> Unit) {
        val char = notifyCharacteristic
        val gatt = bluetoothGatt
        if (char == null || gatt == null) {
            onStatus("当前没有可写特征")
            return
        }

        val frame = BtProtocol.buildTimeSyncFrame()
        char.writeType = BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
        char.value = frame
        val ok = gatt.writeCharacteristic(char)
        onStatus(if (ok) "时间同步请求已发送" else "时间同步写入失败")
    }
}
