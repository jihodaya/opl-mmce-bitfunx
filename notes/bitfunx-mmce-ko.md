# Bitfunx PSxMemCard Gen2 테스트 빌드

[ps2-mmce/Open-PS2-Loader](https://github.com/ps2-mmce/Open-PS2-Loader)의 `mmce_support` 브랜치(`e554796`, 2025-11-24)를 기반으로 수정했습니다. OPL MMCE Beta 2 이후의 공식 OPL 수정(GCC 15 대응, MX4SIO 회귀 수정, VMC 그룹)과 MMCE 알람·대기 사이클 설정 제거가 포함됩니다. 여기에 두 가지를 더했습니다.

- 최신 PS2SDK의 `iopfixup` 요구에 맞춘 공식 OPL 수정 [`656cb1f`](https://github.com/ps2homebrew/Open-PS2-Loader/commit/656cb1f7)
- 공개 PS2SDK와 mmceman에 없는 `mmcesio2.irx` 로드를 빼고 Beta 2처럼 `mmceman.irx`만 로드

이 빌드의 `lang_Korean.lng`는 테스트 빌드 1과 문구 순서가 달라 서로 섞어 쓸 수 없습니다. ELF와 같은 릴리스의 LNG 파일을 함께 사용하세요.

제공된 `sd2psx.uf2`의 SHA-256은 [sd2psXtd 1.4.0 배포본](https://github.com/sd2psXtd/firmware/releases/tag/1.4.0)과 일치합니다.

## 설치 및 확인

1. zip을 풀어 나온 `OPL-Bitfunx-MMCE.ELF`와 `LNG` 폴더를 **같은 폴더에 그대로** 복사합니다. 예를 들어 Gen2 microSD의 `APPS/OPL-Bitfunx/`에 넣으면 됩니다. 기존 OPL을 덮어쓰지 말고 따로 두세요.
2. 이 ELF를 실행하고 OPL에서 **MMCE 시작 모드**와 **응용프로그램 시작 모드**를 켭니다.
3. **설정 → 화면 설정 → 언어**에서 `Korean`을 선택하고 저장합니다. MMCE 게임 목록이 열린 뒤에 목록에 나타납니다.
4. 표지는 `ART/<게임 ID>_COV.png`를 우선 읽고 `.jpg`, `.jpeg`, `_COV`가 없는 이름도 지원합니다. MMCE 기본 폴더를 설정한 경우 그곳의 `ART/`와 카드 루트의 `ART/`를 모두 확인합니다.

### 한국어 파일을 찾는 위치

OPL은 `lang_Korean.lng`와 `font_Korean.ttf`를 다음 위치에서 찾습니다. 두 파일은 항상 같은 폴더에 있어야 합니다.

- ELF가 있는 폴더의 `LNG/` 또는 ELF가 있는 폴더 자체 (테스트 빌드 3부터)
- Gen2 microSD 루트의 `LNG/`, MMCE 기본 폴더를 설정했다면 그 폴더의 `LNG/`
- USB 등 BDM 장치 루트의 `LNG/`
- 메모리카드의 `OPL/` 폴더 바로 안 (`mc0:/OPL/lang_Korean.lng`)

같은 언어 파일이 여러 곳에 있으면 메모리카드 `OPL/`의 파일을 먼저 씁니다. 이전 테스트 빌드의 `lang_Korean.lng`가 메모리카드에 남아 있으면 지우세요. 문구 순서가 달라 메뉴 글자가 엉뚱하게 표시됩니다.

### PS1 게임

PS1은 지금처럼 USB에서 실행하는 방법을 권장합니다. MMCE 실행을 시험하면서 [MMCEMAN for POPSTARTER 테스트 드라이버](https://github.com/ps2-mmce/mmceman/releases/tag/popstarter)의 `usbd.irx`와 `usbhdfsd.irx`를 `mc0:/POPSTARTER/`에 넣었다면, 지우거나 백업해 둔 원래 파일로 되돌려야 USB에서 다시 실행됩니다.

Gen2 microSD의 `POPS/`에 VCD가 있으면 응용프로그램 메뉴에 `[PS1] 게임명` 항목이 보입니다. 이 항목은 위 테스트 드라이버가 있어야 실행되는 실험 기능입니다.

## 소스 및 출처

- OPL 빌드는 PS2SDK, gsKit, PS2SDK Ports의 libpng와 libjpeg를 사용합니다. Linux 또는 WSL에서 `make download_lwNBD`, `make src/lang_internal.c include/lang_autogen.h`, `make opl.elf` 순서로 실행합니다. 패킹은 환경에 맞는 PS2-Packer 스텁을 사용합니다.
- 한국어 번역은 [ddinghoya의 기존 OPL 번역](https://github.com/ps2homebrew/Open-PS2-Loader-lang/blob/b23b743d9f8c65e7a305db945f442e5755019221/Korean.yml)에 [MMCE 문구](../lng_tmpl/Korean-MMCE.yml)를 더했습니다. `make download_lng` 이후 `make lng/lang_Korean.lng`로 생성합니다.
- `font_Korean.ttf`는 [OPL 번역 저장소에서 배포하는 UnTaza](https://github.com/ps2homebrew/Open-PS2-Loader-lang/blob/b23b743d9f8c65e7a305db945f442e5755019221/thirdparty/font_Korean.ttf)입니다. [UnTaza 글꼴 소스와 라이선스](https://ctan.org/tex-archive/fonts/unfonts-extra)를 참고하세요.
