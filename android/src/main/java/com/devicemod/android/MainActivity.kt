package com.devicemod.android

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

private val Bg = Color(0xFF040A12)
private val Panel = Color(0xFF081722)
private val Panel2 = Color(0xFF0B1D2A)
private val Cyan = Color(0xFF19E3FF)
private val Dim = Color(0xFF4A91A0)
private val Amber = Color(0xFFFFB000)
private val Red = Color(0xFFFF5263)
private val Ink = Color(0xFFD6F8FF)
private val Green = Color(0xFF54F0A5)

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent { DeviceModApp() }
    }
}

private data class Module(val title: String, val subtitle: String, val code: String)
private val modules = listOf(
    Module("OVERVIEW", "Device status and operation center", "OVERVIEW"),
    Module("BOOTLOADER", "Unlock preparation and Fastboot", "BOOTLOADER"),
    Module("MAGISK ROOT", "Patch boot image and verify root", "ROOT"),
    Module("ORANGEFOX", "Recovery and misc partition workflow", "RECOVERY"),
    Module("KALI NETHUNTER", "ARM64 chroot installation guide", "NETHUNTER"),
    Module("BOOT ANIMATION", "Prepare and install animation ZIP", "ANIMATION"),
    Module("SPLASH LAB", "Splash image design and compatibility", "SPLASH"),
)

@Composable
private fun DeviceModApp() {
    var selected by remember { mutableIntStateOf(0) }
    var amberMode by remember { mutableStateOf(false) }
    val accent = if (amberMode) Amber else Cyan
    val clipboard = LocalClipboardManager.current

    MaterialTheme {
        Box(Modifier.fillMaxSize().background(Bg)) {
            HudGrid(accent)
            Column(Modifier.fillMaxSize().padding(12.dp)) {
                Header(accent, amberMode) { amberMode = !amberMode }
                Spacer(Modifier.height(10.dp))
                Row(Modifier.weight(1f), horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                    if (androidx.compose.ui.platform.LocalConfiguration.current.screenWidthDp >= 650) {
                        Column(
                            Modifier.width(180.dp).fillMaxHeight(),
                            verticalArrangement = Arrangement.spacedBy(7.dp)
                        ) {
                            modules.forEachIndexed { index, module ->
                                HudButton(
                                    label = module.title,
                                    selected = selected == index,
                                    accent = accent,
                                    modifier = Modifier.fillMaxWidth().height(54.dp)
                                ) { selected = index }
                            }
                            Spacer(Modifier.weight(1f))
                            HudPanel("TARGET PROFILE", accent) {
                                HudText("LENOVO TB-X505X", accent, 11)
                                HudText("SDM429 / ARM64", Dim, 10)
                                HudText("PROFILE: COMMUNITY", Amber, 9)
                            }
                        }
                    }
                    Column(Modifier.weight(1f).fillMaxHeight()) {
                        if (selected == 0) {
                            Overview(accent, onOpen = { selected = it })
                        } else {
                            ModuleScreen(
                                index = selected,
                                accent = accent,
                                onCopy = { clipboard.setText(AnnotatedString(it)) }
                            )
                        }
                    }
                }
                if (androidx.compose.ui.platform.LocalConfiguration.current.screenWidthDp < 650) {
                    Spacer(Modifier.height(8.dp))
                    Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
                        modules.forEachIndexed { index, module ->
                            HudButton(
                                label = module.code,
                                selected = selected == index,
                                accent = accent,
                                modifier = Modifier.weight(1f).height(42.dp),
                                small = true
                            ) { selected = index }
                        }
                    }
                }
                Spacer(Modifier.height(5.dp))
                HudText("DEVICEMOD // LOCAL GUIDE MODE // NO COMMANDS EXECUTED ON THIS DEVICE", Dim, 9)
            }
        }
    }
}

@Composable
private fun Header(accent: Color, amberMode: Boolean, onTheme: () -> Unit) {
    Row(
        Modifier.fillMaxWidth().height(58.dp)
            .border(1.dp, accent.copy(alpha = .45f), RoundedCornerShape(8.dp))
            .background(Panel.copy(alpha = .94f), RoundedCornerShape(8.dp))
            .padding(horizontal = 14.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(Modifier.weight(1f)) {
            HudText("◈ DEVICEMOD", accent, 20, true)
            HudText("ANDROID MODIFICATION CONTROL SYSTEM", Dim, 9)
        }
        HudText("TARGET // X505X", Ink, 10, true)
        Spacer(Modifier.width(12.dp))
        HudButton(if (amberMode) "AMBER" else "CYAN", accent, false, Modifier.width(76.dp).height(36.dp), true, onTheme)
    }
}

@Composable
private fun Overview(accent: Color, onOpen: (Int) -> Unit) {
    LazyColumn(verticalArrangement = Arrangement.spacedBy(9.dp)) {
        item {
            HudPanel("DEVICE PROFILE // TARGET", accent) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Column(Modifier.weight(1f)) {
                        HudText("LENOVO TAB M10 HD", Ink, 20, true)
                        HudText("MODEL: TB-X505X  •  PLATFORM: QUALCOMM SDM429", Dim, 10)
                        Spacer(Modifier.height(9.dp))
                        Row(horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                            StatusChip("ADB / FASTBOOT", "HOST REQUIRED", Amber)
                            StatusChip("BOOTLOADER", "UNKNOWN", Red)
                        }
                    }
                    DeviceGlyph(accent, Modifier.size(100.dp, 126.dp))
                }
            }
        }
        item {
            HudPanel("OPERATION SEQUENCE", accent) {
                HudText("Select a module to view its guided workflow.", Dim, 10)
            }
        }
        itemsIndexed(modules.drop(1)) { i, module ->
            Row(
                Modifier.fillMaxWidth().clip(RoundedCornerShape(7.dp))
                    .background(Panel.copy(alpha = .92f))
                    .border(1.dp, accent.copy(alpha = .32f), RoundedCornerShape(7.dp))
                    .clickable { onOpen(i + 1) }.padding(12.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                HudText("0${i + 1}", accent, 14, true)
                Spacer(Modifier.width(12.dp))
                Column(Modifier.weight(1f)) {
                    HudText(module.title, Ink, 13, true)
                    HudText(module.subtitle, Dim, 10)
                }
                HudText("OPEN  ›", accent, 10, true)
            }
        }
        item {
            HudPanel("SAFETY INTERLOCK", Amber) {
                HudText("Unlocking erases user data. Flashing an image for another model or partition layout can permanently damage the device.", Amber, 10)
                HudText("DeviceMod does not bypass OEM authorization or execute privileged commands from Android.", Dim, 9)
            }
        }
    }
}

@Composable
private fun ModuleScreen(index: Int, accent: Color, onCopy: (String) -> Unit) {
    val title = modules[index].title
    val sections = when (index) {
        1 -> listOf(
            Guide("01 / PREPARE", "Back up all personal data. Bootloader unlocking normally performs a factory reset.", null, Amber),
            Guide("02 / DEVELOPER OPTIONS", "Settings → About tablet → tap Build number seven times. Open Developer options, enable OEM unlocking and USB debugging.", null),
            Guide("03 / CONNECT TO HOST", "Install Android SDK Platform Tools on Windows. Connect the tablet with a data-capable USB cable and accept the USB debugging prompt.", "adb devices"),
            Guide("04 / ENTER FASTBOOT", "Reboot to bootloader. Confirm the tablet is detected before issuing any unlock command.", "adb reboot bootloader"),
            Guide("05 / VERIFY FASTBOOT", "Check the detected device and inspect the product/unlock state. Commands vary by bootloader build.", "fastboot devices\nfastboot getvar product\nfastboot getvar unlocked"),
            Guide("06 / UNLOCK (DESTRUCTIVE)", "Only continue after confirming the exact TB-X505X firmware, a backup, and the device-specific unlock procedure. This command is community-reported, not Lenovo-confirmed.", "fastboot oem unlock-go", Red)
        )
        2 -> listOf(
            Guide("01 / GET STOCK BOOT IMAGE", "Obtain boot.img from the exact firmware build currently installed on the tablet. Never use an image from a different build or model."),
            Guide("02 / PATCH WITH MAGISK", "Install the official Magisk app on the tablet. Select Install → Select and Patch a File, then choose the matching boot.img."),
            Guide("03 / TRANSFER PATCHED IMAGE", "Copy magisk_patched*.img back to the Windows host. Keep an untouched copy of the original boot image."),
            Guide("04 / FLASH ROOT IMAGE", "The exact boot partition name and slot layout must be verified for this firmware before flashing. DeviceMod intentionally does not guess a partition command.", null, Amber),
            Guide("05 / VERIFY ROOT", "Boot Android and verify root access in Magisk. Do not install modules until a clean boot is confirmed.")
        )
        3 -> listOf(
            Guide("01 / VERIFY RECOVERY BUILD", "Use only the OrangeFox build explicitly released for Lenovo TB-X505X. Verify its checksum and release notes."),
            Guide("02 / FLASH RECOVERY", "The provided guide uses the recovery partition. Confirm that this partition exists on your installed firmware.", "fastboot flash recovery recovery.img"),
            Guide("03 / SET RECOVERY ENTRY", "The supplied release notes require its matching misc_rebootrecovery.img.", "fastboot flash misc misc_rebootrecovery.img"),
            Guide("04 / REBOOT", "Do not use fastboot boot for this OrangeFox build.", "fastboot reboot"),
            Guide("05 / SIDeload ZIP", "In OrangeFox: Menu → ADB & Sideload → Swipe to start sideload. Then use:", "adb sideload OrangeFox.zip")
        )
        4 -> listOf(
            Guide("01 / REQUIREMENTS", "Confirm root, supported Android version, ARM64 architecture, sufficient free storage, and the exact NetHunter package variant."),
            Guide("02 / PACKAGE CHECK", "A generic ARM64 Full image is not automatically a flashable recovery ZIP. Follow the package's own installation method and verify its checksum.", null, Amber),
            Guide("03 / INSTALL CHROOT", "Install the matching NetHunter app and chroot package using the official NetHunter documentation. Do not flash a generic image to an unknown partition."),
            Guide("04 / VERIFY", "Check chroot startup, terminal, networking and kernel feature support. A generic userspace image does not add Wi-Fi injection or HID kernel features.")
        )
        5 -> listOf(
            Guide("01 / PREPARE ZIP", "Create a valid Android bootanimation.zip with the expected desc.txt and part folders. Keep dimensions and frame rate appropriate for the display."),
            Guide("02 / BACKUP", "Back up the original bootanimation.zip and verify the replacement file before applying it."),
            Guide("03 / INSTALL", "Prefer a Magisk module or a recovery package designed for this exact Android version. System partition paths differ by release."),
            Guide("04 / VERIFY", "Reboot and check animation timing. If boot fails, restore the original animation through recovery.")
        )
        else -> listOf(
            Guide("01 / IMAGE FORMAT", "Splash screen is a bootloader/vendor-specific image or partition, not the Android bootanimation.zip format."),
            Guide("02 / DEVICE RESEARCH", "Identify the exact splash/logo partition, pixel dimensions, pixel format, offsets and image header for TB-X505X. Do not reuse Redmi Note 6 Pro instructions.", null, Amber),
            Guide("03 / GENERATE", "Prepare a preview image first. A device-specific encoder is required before a binary splash image can be produced."),
            Guide("04 / FLASH LOCK", "Flashing is disabled in this guide until the X505X splash partition and image format are verified. Never guess a partition name or raw image layout.", null, Red)
        )
    }

    Column(Modifier.fillMaxSize()) {
        HudPanel("MODULE // $title", accent) {
            HudText("GUIDED WORKFLOW  •  FOLLOW IN ORDER", Dim, 10)
        }
        Spacer(Modifier.height(8.dp))
        LazyColumn(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            itemsIndexed(sections) { i, guide ->
                GuideCard(i + 1, guide, accent, onCopy)
            }
            item {
                HudPanel("HOST REQUIREMENT", accent) {
                    HudText("USB / ADB / FASTBOOT actions are performed by the Windows companion application. This Android app is the on-device checklist and reference guide.", Dim, 10)
                }
            }
        }
    }
}

private data class Guide(val title: String, val body: String, val command: String? = null, val tone: Color? = null)

@Composable
private fun GuideCard(number: Int, guide: Guide, accent: Color, onCopy: (String) -> Unit) {
    val tone = guide.tone ?: accent
    HudPanel(guide.title, tone) {
        HudText(guide.body, Ink, 11)
        guide.command?.let {
            Spacer(Modifier.height(8.dp))
            Box(
                Modifier.fillMaxWidth().background(Color(0xFF02070D), RoundedCornerShape(5.dp))
                    .border(1.dp, tone.copy(alpha = .35f), RoundedCornerShape(5.dp)).padding(9.dp)
            ) {
                Column {
                    HudText(it, Color(0xFFB6F7FF), 10)
                    Spacer(Modifier.height(6.dp))
                    HudButton("COPY COMMAND", tone, false, Modifier.fillMaxWidth().height(34.dp), true) { onCopy(it) }
                }
            }
        }
    }
}

@Composable
private fun StatusChip(label: String, value: String, color: Color) {
    Column(Modifier.background(color.copy(alpha = .08f), RoundedCornerShape(4.dp))
        .border(1.dp, color.copy(alpha = .5f), RoundedCornerShape(4.dp)).padding(7.dp)) {
        HudText(label, Dim, 8)
        HudText(value, color, 9, true)
    }
}

@Composable
private fun HudPanel(title: String, accent: Color, content: @Composable ColumnScope.() -> Unit) {
    Column(
        Modifier.fillMaxWidth().background(Panel.copy(alpha = .95f), RoundedCornerShape(8.dp))
            .border(1.dp, accent.copy(alpha = .45f), RoundedCornerShape(8.dp))
            .padding(11.dp)
    ) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Box(Modifier.size(4.dp, 13.dp).background(Amber))
            Spacer(Modifier.width(7.dp))
            HudText(title, accent, 10, true)
        }
        Spacer(Modifier.height(8.dp))
        content()
    }
}

@Composable
private fun HudButton(
    label: String,
    accent: Color,
    selected: Boolean,
    modifier: Modifier = Modifier,
    small: Boolean = false,
    onClick: () -> Unit
) {
    Box(
        modifier.clip(RoundedCornerShape(6.dp))
            .background(if (selected) accent.copy(alpha = .18f) else Panel2.copy(alpha = .78f))
            .border(1.dp, accent.copy(alpha = if (selected) .9f else .4f), RoundedCornerShape(6.dp))
            .clickable(onClick = onClick).padding(horizontal = 6.dp, vertical = 5.dp),
        contentAlignment = Alignment.Center
    ) {
        HudText(label, accent, if (small) 8 else 10, true, maxLines = 2)
    }
}

@Composable
private fun HudText(
    value: String,
    color: Color = Ink,
    size: Int = 12,
    bold: Boolean = false,
    maxLines: Int = Int.MAX_VALUE
) {
    Text(
        value,
        color = color,
        fontSize = size.sp,
        fontFamily = FontFamily.Monospace,
        fontWeight = if (bold) FontWeight.Bold else FontWeight.Normal,
        maxLines = maxLines,
        overflow = TextOverflow.Ellipsis
    )
}

@Composable
private fun DeviceGlyph(accent: Color, modifier: Modifier = Modifier) {
    val transition = rememberInfiniteTransition(label = "device-radar")
    val pulse by transition.animateFloat(
        0.65f, 1f,
        infiniteRepeatable(tween(1800, easing = LinearEasing), RepeatMode.Reverse),
        label = "pulse"
    )
    Canvas(modifier) {
        val w = size.width
        val h = size.height
        drawRoundRect(accent.copy(alpha = .18f), Offset(w * .16f, h * .03f), Size(w * .68f, h * .94f), cornerRadius = androidx.compose.ui.geometry.CornerRadius(18f), style = Stroke(2.dp.toPx()))
        drawRoundRect(accent.copy(alpha = .8f), Offset(w * .21f, h * .08f), Size(w * .58f, h * .82f), cornerRadius = androidx.compose.ui.geometry.CornerRadius(12f), style = Stroke(1.dp.toPx()))
        drawCircle(accent.copy(alpha = .14f * pulse), w * .31f, Offset(w * .5f, h * .45f))
        drawCircle(accent.copy(alpha = .65f), w * .25f, Offset(w * .5f, h * .45f), style = Stroke(1.dp.toPx()))
        drawCircle(accent, w * .035f, Offset(w * .5f, h * .45f))
        drawLine(accent.copy(alpha = .5f), Offset(w * .24f, h * .67f), Offset(w * .76f, h * .67f), 1.dp.toPx())
        drawLine(accent.copy(alpha = .5f), Offset(w * .24f, h * .73f), Offset(w * .62f, h * .73f), 1.dp.toPx())
    }
}

@Composable
private fun HudGrid(accent: Color) {
    val transition = rememberInfiniteTransition(label = "grid-scan")
    val scan by transition.animateFloat(
        0f, 1f, infiniteRepeatable(tween(5000, easing = LinearEasing)), label = "scan"
    )
    Canvas(Modifier.fillMaxSize()) {
        val step = 30.dp.toPx()
        var x = 0f
        while (x < size.width) {
            drawLine(accent.copy(alpha = .045f), Offset(x, 0f), Offset(x, size.height), 1f)
            x += step
        }
        var y = 0f
        while (y < size.height) {
            drawLine(accent.copy(alpha = .045f), Offset(0f, y), Offset(size.width, y), 1f)
            y += step
        }
        val scanY = size.height * scan
        drawLine(accent.copy(alpha = .09f), Offset(0f, scanY), Offset(size.width, scanY), 2.dp.toPx())
    }
}
