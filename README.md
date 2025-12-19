<a id="readme-top"></a>
<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/MohamedTarek3M/Roco-Project">
<!--    <img src="images/logo.png" alt="Logo" width="80" height="80"> -->
  </a>

  <h3 align="center">Roco Project</h3>

  <p align="center">
    This repository contains the comprehensive source code for the Roco Project, including algorithms for movement and data processing.
    <br />
    <a href="https://github.com/MohamedTarek3M/Roco-Project/issues/new?labels=bug&template=bug-report---.md">Report Bug</a>
    &middot;
    <a href="https://github.com/MohamedTarek3M/Roco-Project/issues/new?labels=enhancement&template=feature-request---.md">Request Feature</a>
  </p>
</div>

<!-- GETTING STARTED -->
## Getting Started

Follow these steps.

### Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software)
- ESP32 Board Support
- [PS4Controller Library](https://github.com/aed3/PS4-esp32)

### Features

- **PS4 Control**: Full control using DualShock 4 controller.
- **Speed Control**: Toggle smooth speed limiting with `Cross` button.
- **Boost Mode**: Activate temporary speed boost with `L1 + R1` (3 seconds duration).
- **Rumble Feedback**: Haptic feedback for engine RPM, boost, and cooldown events.
- **LED Indicators**: Visual feedback for modes (Green: Speed Control, Red: Boost, Cyan: Idle).
- **Smooth Acceleration**: Ramping logic for smooth starts and stops.

### Installation

#### 1. Download and Install the Arduino IDE

Download the latest version of the Arduino IDE from the official site:

➡️ [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

Install it according to your operating system.

#### 2. Install the ESP32 Board in Arduino IDE

1. Open the Arduino IDE.
2. Go to **File > Preferences**.
3. In the **"Additional Board Manager URLs"** field, add the following URL:
```sh
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
4. Click **OK** to save.
5. Go to **Tools > Board > Boards Manager**.
6. Search for **"ESP32"**, then click **Install** on the package developed by Espressif Systems.
7. Once installed, go to **Tools > Board** and select your ESP32 board model (e.g., *ESP32 Dev Module*).

#### 3. Install Libraries

1. Go to **Sketch > Include Library > Manage Libraries**.
2. Search for `PS4Controller` and install the library by `aed3`.

#### 4. Upload the Code

1. Connect your ESP32 to your PC via USB.
2. Open the project `.ino` file in Arduino IDE.
3. Select the correct Port and Board under Tools.
4. Click the Upload button to flash the code onto your ESP32.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
<!--
## Usage



_For more examples, please refer to the [Documentation](https://)_

<p align="right">(<a href="#readme-top">back to top</a>)</p>
-->



<!-- ROADMAP -->
## Roadmap

- [ ] Add Changelog
<!--
    - [x] New
    - [ ] Update
-->

See the [open issues](https://github.com/MohamedTarek3M/Roco-Project/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>


## FAQ

❓ What if the ESP32 board doesn’t show up in the Arduino IDE?

- Make sure you have installed the correct USB drivers.

- Try a different USB cable or port.

- Restart the Arduino IDE after installing the ESP32 board package.

❓ How do I reset the ESP32 if it fails to upload?

- Hold the `BOOT` button while clicking the `Upload` button in Arduino IDE.

- Release the `BOOT` button when it starts connecting.

❓ Can I use a different board?

- The code is written for the ESP32 platform. Other boards would require code modifications.

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- LICENSE -->
<!--
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>
-->


<!-- CONTACT -->
## Contact

Mohamed Tarek - [@MohamedTarek3M](https://twitter.com/MohamedTarek3M) - mohamedtarekcontact@gmail.com

Project Link: [https://github.com/MohamedTarek3M/Roco-Project](https://github.com/MohamedTarek3M/Roco-Project)

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
<!--
[issues-url]: https://github.com/MohamedTarek3M/Roco-Project/issues
[license-url]: https://github.com/MohamedTarek3M/Roco-Project/blob/master/LICENSE.txt
[product-screenshot]: images/screenshot.png
-->
