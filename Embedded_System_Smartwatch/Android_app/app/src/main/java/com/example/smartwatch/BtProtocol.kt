package com.example.smartwatch

import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.time.LocalDateTime

object BtProtocol {
    private const val STX: Byte = 0xAA.toByte()
    private const val ETX: Byte = 0x55.toByte()
    private const val CMD_SENSOR_DATA: Byte = 0x01
    private const val CMD_TIME_SYNC: Byte = 0x02
    private const val CMD_DEVICE_STATUS: Byte = 0x04

    fun parseFrame(raw: ByteArray): ParsedFrame? {
        if (raw.size < 6) return null
        if (raw[0] != STX || raw[raw.size - 1] != ETX) return null

        val cmd = raw[1]
        val len = raw[2].toInt() and 0xFF
        if (raw.size != len + 5) return null

        val data = raw.sliceArray(3 until 3 + len)
        val checksum = raw[3 + len]
        if (checksum != calcChecksum(cmd, len, data)) return null

        return when (cmd) {
            CMD_SENSOR_DATA -> parseSensorData(data)
            CMD_TIME_SYNC -> ParsedFrame.TimeSyncResponse(data)
            CMD_DEVICE_STATUS -> parseDeviceStatus(data)
            else -> ParsedFrame.Unknown(cmd, data)
        }
    }

    private fun calcChecksum(cmd: Byte, len: Int, data: ByteArray): Byte {
        var chk = cmd.toInt()
        chk = chk xor len
        for (b in data) chk = chk xor b.toInt()
        return chk.toByte()
    }

    private fun parseSensorData(data: ByteArray): ParsedFrame {
        if (data.size != 24) return ParsedFrame.Invalid("Sensor payload length mismatch")
        val buffer = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN)
        val ax = buffer.float
        val ay = buffer.float
        val az = buffer.float
        val gx = buffer.float
        val gy = buffer.float
        val gz = buffer.float
        return ParsedFrame.Sensors(ax, ay, az, gx, gy, gz)
    }

    /* DeviceStatus payload: byte 0-7 = time, 8-11 = step, 12-15 = distance, 16-19 = calories */
    private fun parseDeviceStatus(data: ByteArray): ParsedFrame {
        if (data.size < 8) return ParsedFrame.Invalid("Device status payload too short")
        val hour = data[0].toInt() and 0xFF
        val minute = data[1].toInt() and 0xFF
        val second = data[2].toInt() and 0xFF
        val year = ((data[3].toInt() and 0xFF) shl 8) or (data[4].toInt() and 0xFF)
        val month = data[5].toInt() and 0xFF
        val day = data[6].toInt() and 0xFF
        val weekday = data[7].toInt() and 0xFF

        var step = 0L
        var distance = 0f
        var calories = 0f
        if (data.size >= 12) {
            val buf = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN)
            buf.position(8)
            step = buf.int.toLong() and 0xFFFFFFFFL
        }
        if (data.size >= 16) {
            val buf = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN)
            buf.position(12)
            distance = buf.float
        }
        if (data.size >= 20) {
            val buf = ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN)
            buf.position(16)
            calories = buf.float
        }
        return ParsedFrame.DeviceStatus(hour, minute, second, year, month, day, weekday, step, distance, calories)
    }

    fun buildTimeSyncFrame(): ByteArray {
        val now = LocalDateTime.now()
        val payload = byteArrayOf(
            now.hour.toByte(),
            now.minute.toByte(),
            now.second.toByte(),
            ((now.year shr 8) and 0xFF).toByte(),
            (now.year and 0xFF).toByte(),
            now.monthValue.toByte(),
            now.dayOfMonth.toByte()
        )
        val frame = ByteArray(payload.size + 5)
        frame[0] = STX
        frame[1] = CMD_TIME_SYNC
        frame[2] = payload.size.toByte()
        System.arraycopy(payload, 0, frame, 3, payload.size)
        frame[3 + payload.size] = calcChecksum(CMD_TIME_SYNC, payload.size, payload)
        frame[4 + payload.size] = ETX
        return frame
    }

    sealed class ParsedFrame {
        data class Sensors(val ax: Float, val ay: Float, val az: Float, val gx: Float, val gy: Float, val gz: Float) : ParsedFrame()
        data class DeviceStatus(
            val hour: Int, val minute: Int, val second: Int,
            val year: Int, val month: Int, val day: Int, val weekday: Int,
            val steps: Long, val distance: Float, val calories: Float
        ) : ParsedFrame()
        data class TimeSyncResponse(val data: ByteArray) : ParsedFrame()
        data class Unknown(val cmd: Byte, val data: ByteArray) : ParsedFrame()
        data class Invalid(val reason: String) : ParsedFrame()
    }
}