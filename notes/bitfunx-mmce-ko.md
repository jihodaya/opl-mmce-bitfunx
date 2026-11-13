# Bitfunx PSxMemCard Gen2 테스트 빌드

[ps2-mmce/Open-PS2-Loader](https://github.com/ps2-mmce/Open-PS2-Loader)의 `mmce_support` 브랜치(`e554796`, 2025-11-24)를 기반으로 수정했습니다. OPL MMCE Beta 2 이후의 공식 OPL 수정(GCC 15 대응, MX4SIO 회귀 수정, VMC 그룹)과 MMCE 알람·대기 사이클 설정 제거가 포함됩니다. 여기에 두 가지를 더했습니다.

- 최신 PS2SDK의 `iopfixup` 요구에 맞춘 공식 OPL 수정 [`656cb1f`](https://github.com/ps2homebrew/Open-PS2-Loader/commit/656cb1f7)
- 공개 PS2SDK와 mmceman에 없는 `mmcesio2.irx` 로드를 빼고 Beta 2처럼 `mmceman.irx`만 로드

이 빌드의 `lang_Korean.lng`는 테스트 빌드 1과 문구 순서가 달라 서로 섞어 쓸 수 없습니다. ELF와 같은 릴리스의 LNG 파일을 함께 사용하세요.

제공된 `sd2psx.uf2`의 SHA-256은 [sd2psXtd 1.4.0 배포본](https://github.com/sd2psXtd/firmware/releases/tag/1.4.0)과 일치합니다.

## 설치 및 확인

1. 기존 OPL과 함께 `OPL-Bitfunx-MMCE.ELF`를 별도 이름으로 두고 실행합니다. OPL에서 **MMCE 시작 모드**와 **응용프로그램 시작 모드**를 켭니다.
2. 한국어를 사용하려면 `LNG/lang_Korean.lng`와 `LNG/font_Korean.ttf`를 Gen2의 `LNG/` 폴더에 넣습니다. MMCE 기본 폴더를 설정했다면 그 폴더의 `LNG/`를 사용합니다. 초기 부팅부터 한국어를 쓰려면 두 파일을 메모리카드의 `OPL/` 폴더에 넣고, OPL 화면 설정에서 `Korean`을 선택하고 저장합니다.
3. 표지만 있는 PS2 게임을 확인합니다. `ART/<게임 ID>_COV.png`를 우선 읽고 `.jpg`, `.jpeg`, `_COV`가 없는 이름도 지원합니다. MMCE 기본 폴더를 설정한 경우 그곳의 `ART/`와 카드 루트의 `ART/`를 모두 확인합니다.
4. PS1 실행을 확인하려면 Gen2를 PS2 **슬롯 1**에 두고, microSD 루트의 `POPS/`에 `POPSTARTER.ELF`, `게임명.VCD`, `POPS_IOX.PAK`를 둡니다. 사용 중인 POPStarter 빌드에 추가 지원 파일이 있다면 USB에서 정상 작동하던 구성도 함께 복사합니다.
5. [MMCEMAN for POPSTARTER 테스트 드라이버](https://github.com/ps2-mmce/mmceman/releases/tag/popstarter)의 `usbd.irx`와 `usbhdfsd.irx`를 PS2 메모리카드의 `mc0:/POPSTARTER/`에 넣습니다. 이 폴더는 microSD의 루트 `POPS/`와 별개입니다. USB 또는 MX4SIO에서 사용한 IRX가 있으면 먼저 복사해 두고, MMCE 테스트가 끝난 뒤 해당 구성을 쓸 때 복원합니다.
6. USB를 빼고 응용프로그램 메뉴의 `[PS1] 게임명`을 실행합니다. OPL은 ELF를 MMCE에서 읽고, POPStarter에는 드라이버가 다시 마운트할 `mass:/POPS/XX.게임명.ELF` 경로를 넘깁니다. 이 경로에 맞춘 게임별 ELF 복사본은 필요 없습니다. 기존 `mmce0:`의 `XX.*.ELF` 앱 항목도 같은 경로로 실행됩니다.

실기에서 확인할 항목은 표지만 있는 PS2 게임, FAT32 또는 exFAT Gen2의 PS1 부팅과 저장, 기존 USB 및 MX4SIO 실행입니다. 검은 화면이 재현되면 VCD 이름, POPStarter 버전, 파일시스템, 표시된 OPL 안내가 다음 진단에 필요합니다.

## 소스 및 출처

- OPL 빌드는 PS2SDK, gsKit, PS2SDK Ports의 libpng와 libjpeg를 사용합니다. Linux 또는 WSL에서 `make download_lwNBD`, `make src/lang_internal.c include/lang_autogen.h`, `make opl.elf` 순서로 실행합니다. 패킹은 환경에 맞는 PS2-Packer 스텁을 사용합니다.
- 한국어 번역은 [ddinghoya의 기존 OPL 번역](https://github.com/ps2homebrew/Open-PS2-Loader-lang/blob/b23b743d9f8c65e7a305db945f442e5755019221/Korean.yml)에 [MMCE 문구](../lng_tmpl/Korean-MMCE.yml)를 더했습니다. `make download_lng` 이후 `make lng/lang_Korean.lng`로 생성합니다.
- `font_Korean.ttf`는 [OPL 번역 저장소에서 배포하는 UnTaza](https://github.com/ps2homebrew/Open-PS2-Loader-lang/blob/b23b743d9f8c65e7a305db945f442e5755019221/thirdparty/font_Korean.ttf)입니다. [UnTaza 글꼴 소스와 라이선스](https://ctan.org/tex-archive/fonts/unfonts-extra)를 참고하세요.
