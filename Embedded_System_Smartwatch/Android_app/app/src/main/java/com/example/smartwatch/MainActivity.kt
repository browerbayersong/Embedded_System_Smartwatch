package com.example.smartwatch

import android.Manifest
import android.bluetooth.BluetoothDevice
import android.content.Context
import android.os.Build
import android.os.Bundle
import android.os.Environment
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts.RequestMultiplePermissions
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Divider
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.runtime.snapshots.SnapshotStateList
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.smartwatch.ui.theme.SmartWatchTheme
import java.io.File
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class MainActivity : ComponentActivity() {
    private lateinit var bleManager: BleManager

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        bleManager = BleManager(this)

        setContent {
            val context = LocalContext.current

            // 全局状态
            val deviceList = remember { mutableStateListOf<BluetoothDevice>() }
            val logLines = remember { mutableStateListOf<String>() }
            val connectionState = remember { mutableStateOf("未连接") }
            val sensorText = remember { mutableStateOf("无数据") }
            val deviceTimeText = remember { mutableStateOf("--:--:--") }
            val connectedDevice = remember { mutableStateOf<BluetoothDevice?>(null) }

            val permissionState = remember { mutableStateOf(false) }
            val darkTheme = remember { mutableStateOf(true) }
            val primaryColor = remember { mutableStateOf(Color(0xFF4CAF50)) }
            val currentPage = remember { mutableStateOf(0) }

            val permissionLauncher = rememberLauncherForActivityResult(RequestMultiplePermissions()) { grantResults ->
                permissionState.value = grantResults.values.all { it }
                if (!permissionState.value) {
                    logLines.add("请授予蓝牙权限")
                }
            }

            LaunchedEffect(Unit) {
                val permissions = mutableListOf<String>()
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    permissions += Manifest.permission.BLUETOOTH_SCAN
                    permissions += Manifest.permission.BLUETOOTH_CONNECT
                }
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                    permissions += Manifest.permission.ACCESS_FINE_LOCATION
                }
                if (permissions.isNotEmpty()) {
                    permissionLauncher.launch(permissions.toTypedArray())
                } else {
                    permissionState.value = true
                }
            }

            SmartWatchTheme(
                darkTheme = darkTheme.value,
                primaryColor = primaryColor.value
            ) {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    AppScaffold(
                        page = currentPage.value,
                        onPageChange = { currentPage.value = it },
                        bleManager = bleManager,
                        deviceList = deviceList,
                        logLines = logLines,
                        connectionState = connectionState,
                        sensorText = sensorText,
                        deviceTimeText = deviceTimeText,
                        connectedDevice = connectedDevice,
                        permissionState = permissionState.value,
                        darkTheme = darkTheme.value,
                        onDarkThemeChange = { darkTheme.value = it },
                        primaryColor = primaryColor.value,
                        onPrimaryColorChange = { primaryColor.value = it },
                        context = context
                    )
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AppScaffold(
    page: Int,
    onPageChange: (Int) -> Unit,
    bleManager: BleManager,
    deviceList: SnapshotStateList<BluetoothDevice>,
    logLines: SnapshotStateList<String>,
    connectionState: androidx.compose.runtime.MutableState<String>,
    sensorText: androidx.compose.runtime.MutableState<String>,
    deviceTimeText: androidx.compose.runtime.MutableState<String>,
    connectedDevice: androidx.compose.runtime.MutableState<BluetoothDevice?>,
    permissionState: Boolean,
    darkTheme: Boolean,
    onDarkThemeChange: (Boolean) -> Unit,
    primaryColor: Color,
    onPrimaryColorChange: (Color) -> Unit,
    context: Context
) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = when (page) {
                            0 -> "主页"
                            1 -> "连接"
                            else -> "设置"
                        },
                        fontWeight = FontWeight.Bold
                    )
                },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = MaterialTheme.colorScheme.primary,
                    titleContentColor = Color.White
                )
            )
        },
        bottomBar = {
            NavigationBar {
                NavigationBarItem(
                    selected = page == 0,
                    onClick = { onPageChange(0) },
                    icon = { Text("🏠", fontSize = 22.sp) },
                    label = { Text("主页") }
                )
                NavigationBarItem(
                    selected = page == 1,
                    onClick = { onPageChange(1) },
                    icon = { Text("🔗", fontSize = 22.sp) },
                    label = { Text("连接") }
                )
                NavigationBarItem(
                    selected = page == 2,
                    onClick = { onPageChange(2) },
                    icon = { Text("⚙", fontSize = 22.sp) },
                    label = { Text("设置") }
                )
            }
        }
    ) { innerPadding ->
        Box(
            modifier = Modifier
                .fillMaxSize()
                .padding(innerPadding)
        ) {
            when (page) {
                0 -> HomePage(
                    bleManager = bleManager,
                    logLines = logLines,
                    connectionState = connectionState.value,
                    sensorText = sensorText.value,
                    deviceTimeText = deviceTimeText.value,
                    connectedDevice = connectedDevice.value
                )
                1 -> ConnectionPage(
                    bleManager = bleManager,
                    deviceList = deviceList,
                    logLines = logLines,
                    connectionState = connectionState,
                    sensorText = sensorText,
                    deviceTimeText = deviceTimeText,
                    connectedDevice = connectedDevice,
                    permissionState = permissionState
                )
                2 -> SettingsPage(
                    logLines = logLines,
                    darkTheme = darkTheme,
                    onDarkThemeChange = onDarkThemeChange,
                    primaryColor = primaryColor,
                    onPrimaryColorChange = onPrimaryColorChange,
                    context = context
                )
            }
        }
    }
}

// ==================== 主页 ====================
@Composable
fun HomePage(
    bleManager: BleManager,
    logLines: SnapshotStateList<String>,
    connectionState: String,
    sensorText: String,
    deviceTimeText: String,
    connectedDevice: BluetoothDevice?
) {
    val phoneTime = remember { mutableStateOf(getCurrentTime()) }
    LaunchedEffect(Unit) {
        while (true) {
            phoneTime.value = getCurrentTime()
            kotlinx.coroutines.delay(1000)
        }
    }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        // 连接状态
        InfoCard("连接状态", if (connectedDevice != null) "已连接" else connectionState)

        // 当前设备
        InfoCard(
            "已连接设备",
            if (connectedDevice != null)
                "${connectedDevice.name ?: "未知设备"}\n${connectedDevice.address}"
            else "未连接任何设备"
        )

        // 时间区域
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            InfoCard("设备时间", deviceTimeText, Modifier.weight(1f))
            InfoCard("手机时间", phoneTime.value, Modifier.weight(1f))
        }

        // 同步按钮
        Button(
            onClick = {
                bleManager.sendTimeSync { status ->
                    logLines.add("[时间同步] $status")
                }
            },
            modifier = Modifier.fillMaxWidth(),
            enabled = connectedDevice != null
        ) {
            Text("同步时间到设备")
        }

        // 传感器数据
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(8.dp),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.surfaceVariant
            )
        ) {
            Column(modifier = Modifier.padding(12.dp)) {
                Text(
                    text = "传感器数据",
                    fontWeight = FontWeight.Bold,
                    fontSize = 16.sp
                )
                Spacer(modifier = Modifier.height(6.dp))
                Text(text = sensorText, fontSize = 13.sp)
            }
        }

        // 设备数据（原始日志最近几条）
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .height(180.dp),
            shape = RoundedCornerShape(8.dp),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.surfaceVariant
            )
        ) {
            Column(modifier = Modifier.padding(12.dp).fillMaxSize()) {
                Text(
                    text = "设备数据",
                    fontWeight = FontWeight.Bold,
                    fontSize = 16.sp
                )
                Spacer(modifier = Modifier.height(6.dp))
                if (logLines.isEmpty()) {
                    Text("(暂无数据)", color = Color.Gray, fontSize = 12.sp)
                } else {
                    val lastLines = logLines.takeLast(20)
                    LazyColumn(modifier = Modifier.fillMaxSize()) {
                        items(lastLines) { line ->
                            Text(line, fontSize = 11.sp)
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun InfoCard(title: String, value: String, modifier: Modifier = Modifier) {
    Card(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(8.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Text(text = title, fontWeight = FontWeight.Bold, fontSize = 14.sp)
            Spacer(modifier = Modifier.height(4.dp))
            Text(text = value, fontSize = 14.sp)
        }
    }
}

fun getCurrentTime(): String {
    return SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date())
}

// ==================== 连接页 ====================
@Composable
fun ConnectionPage(
    bleManager: BleManager,
    deviceList: SnapshotStateList<BluetoothDevice>,
    logLines: SnapshotStateList<String>,
    connectionState: androidx.compose.runtime.MutableState<String>,
    sensorText: androidx.compose.runtime.MutableState<String>,
    deviceTimeText: androidx.compose.runtime.MutableState<String>,
    connectedDevice: androidx.compose.runtime.MutableState<BluetoothDevice?>,
    permissionState: Boolean
) {
    val savedDevices = remember { mutableStateListOf<String>() }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        // 扫描按钮
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Button(
                onClick = {
                    if (permissionState) {
                        logLines.add("开始扫描 BLE 设备")
                        bleManager.startScan(
                            onDeviceFound = { device ->
                                if (!deviceList.any { it.address == device.address }) {
                                    deviceList.add(device)
                                    logLines.add("发现: ${device.name ?: "未知"} ${device.address}")
                                }
                            },
                            onScanStatus = { status -> logLines.add(status) }
                        )
                    } else {
                        logLines.add("缺少蓝牙权限")
                    }
                },
                modifier = Modifier.weight(1f)
            ) { Text("扫描设备") }
            Button(
                onClick = {
                    bleManager.stopScan()
                    logLines.add("停止扫描")
                },
                modifier = Modifier.weight(1f)
            ) { Text("停止扫描") }
        }

        // 已连接
        DeviceSection("已连接设备", listOfNotNull(connectedDevice.value)) { device ->
            OutlinedButton(onClick = {
                bleManager.disconnect()
                connectedDevice.value = null
                connectionState.value = "已断开"
                logLines.add("已断开连接")
            }) { Text("断开") }
        }

        // 已保存（用 MAC 地址列表模拟）
        DeviceSection("已保存设备", emptyList()) { _ -> /* 暂无 */ }

        // 扫描到的
        DeviceSection("扫描到的设备", deviceList) { device ->
            TextButton(onClick = {
                bleManager.connect(device,
                    onConnectionStatus = { status ->
                        connectionState.value = status
                        logLines.add(status)
                        if (status.contains("成功") || status.contains("已连接")) {
                            connectedDevice.value = device
                            if (!savedDevices.contains(device.address)) {
                                savedDevices.add(device.address)
                            }
                        }
                    },
                    onRawData = { raw ->
                        logLines.add("rx ${raw.joinToString(" ") { String.format("%02X", it) }}")
                        val parsed = BtProtocol.parseFrame(raw)
                        when (parsed) {
                            is BtProtocol.ParsedFrame.Sensors -> {
                                sensorText.value = "ax=${parsed.ax.format(2)}, ay=${parsed.ay.format(2)}, az=${parsed.az.format(2)}\n" +
                                    "gx=${parsed.gx.format(2)}, gy=${parsed.gy.format(2)}, gz=${parsed.gz.format(2)}"
                            }
                            is BtProtocol.ParsedFrame.TimeSyncResponse -> {
                                val t = raw.joinToString(" ") { String.format("%02X", it) }
                                deviceTimeText.value = t
                            }
                            is BtProtocol.ParsedFrame.Invalid -> {
                                logLines.add("解析失败: ${parsed.reason}")
                            }
                            is BtProtocol.ParsedFrame.Unknown -> {
                                logLines.add("未知命令: ${parsed.cmd}")
                            }
                            else -> {}
                        }
                    }
                )
            }) { Text("连接") }
        }
    }
}

@Composable
fun DeviceSection(
    title: String,
    devices: List<BluetoothDevice>,
    actionContent: @Composable (BluetoothDevice) -> Unit
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .height(160.dp),
        shape = RoundedCornerShape(8.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column(modifier = Modifier.padding(10.dp).fillMaxSize()) {
            Text(text = title, fontWeight = FontWeight.Bold, fontSize = 15.sp)
            Spacer(modifier = Modifier.height(6.dp))
            if (devices.isEmpty()) {
                Text("(无)", color = Color.Gray, fontSize = 12.sp)
            } else {
                LazyColumn(modifier = Modifier.fillMaxSize()) {
                    items(devices) { device ->
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(vertical = 6.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text(
                                text = "${device.name ?: "未知"} (${device.address})",
                                modifier = Modifier.weight(1f),
                                fontSize = 13.sp
                            )
                            actionContent(device)
                        }
                    }
                }
            }
        }
    }
}

// ==================== 设置页 ====================
@Composable
fun SettingsPage(
    logLines: SnapshotStateList<String>,
    darkTheme: Boolean,
    onDarkThemeChange: (Boolean) -> Unit,
    primaryColor: Color,
    onPrimaryColorChange: (Color) -> Unit,
    context: Context
) {
    val presetColors = listOf(
        Color(0xFF4CAF50) to "绿",
        Color(0xFF2196F3) to "蓝",
        Color(0xFFFF9800) to "橙",
        Color(0xFFE91E63) to "粉",
        Color(0xFF9C27B0) to "紫",
        Color(0xFF795548) to "棕"
    )

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(14.dp)
    ) {
        // 深色/浅色
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text("深色模式", fontSize = 16.sp, modifier = Modifier.weight(1f))
            Switch(checked = darkTheme, onCheckedChange = onDarkThemeChange)
        }

        Divider()

        // 按钮颜色
        Text("按钮颜色", fontSize = 16.sp, fontWeight = FontWeight.Bold)
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            presetColors.forEach { (color, label) ->
                Button(
                    onClick = { onPrimaryColorChange(color) },
                    modifier = Modifier.size(56.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = color
                    ),
                    contentPadding = androidx.compose.foundation.layout.PaddingValues(0.dp)
                ) {
                    Text(label, fontSize = 11.sp)
                }
            }
        }

        Divider()

        // 日志查看/导出
        Text("日志", fontSize = 16.sp, fontWeight = FontWeight.Bold)

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Button(
                onClick = { exportLogs(context, logLines) },
                modifier = Modifier.weight(1f)
            ) { Text("导出日志") }
            Button(
                onClick = { logLines.clear() },
                modifier = Modifier.weight(1f)
            ) { Text("清空日志") }
        }

        Card(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxSize(),
            shape = RoundedCornerShape(8.dp),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.surfaceVariant
            )
        ) {
            Column(modifier = Modifier.padding(10.dp).fillMaxSize()) {
                Text("日志内容 (共 ${logLines.size} 条)", fontSize = 13.sp, fontWeight = FontWeight.Bold)
                Spacer(modifier = Modifier.height(6.dp))
                if (logLines.isEmpty()) {
                    Text("(暂无日志)", color = Color.Gray, fontSize = 12.sp)
                } else {
                    LazyColumn(modifier = Modifier.fillMaxSize()) {
                        items(logLines) { line ->
                            Text(line, fontSize = 11.sp)
                        }
                    }
                }
            }
        }
    }
}

fun exportLogs(context: Context, logLines: SnapshotStateList<String>) {
    try {
        val filename = "ble_log_${SimpleDateFormat("yyyyMMdd_HHmmss", Locale.getDefault()).format(Date())}.txt"
        val dir = context.getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS)
        val file = File(dir, filename)
        file.writeText(logLines.joinToString("\n"))
        logLines.add("日志已导出: ${file.absolutePath}")
    } catch (e: Exception) {
        logLines.add("导出失败: ${e.message}")
    }
}

private fun Float.format(digits: Int) = String.format("%.${digits}f", this)