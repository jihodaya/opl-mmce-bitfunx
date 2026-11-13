# JJome OPL

Bitfunx PSxMemCard Gen2(MMCE)에서 PS2 게임을 편하게 쓰도록 고친 [Open PS2 Loader](https://github.com/ps2homebrew/Open-PS2-Loader)(OPL)입니다. 한국어 메뉴를 넣고, Gen2 microSD에 있는 게임의 표지와 한국어 파일을 더 잘 찾도록 했습니다. OPL 정보 화면에는 `JJome OPL <버전>`으로 표시됩니다.

> [!NOTE]
> 아직 테스트 빌드입니다. 지금 배포하는 버전은 [테스트 빌드 6](https://github.com/jihodaya/opl-mmce-bitfunx/releases/tag/jjome-opl-test-6)이고, 정보 화면에는 `JJome OPL v1.2.0-Beta-2253-7c791de`로 나옵니다.

**[내려받기 (릴리스)](https://github.com/jihodaya/opl-mmce-bitfunx/releases)**

## 주요 기능

- **한국어 메뉴**: 기존 OPL 한국어 번역에 빠져 있던 MMCE 메뉴와 화면 설정 문구를 더했습니다. 한국어 글꼴(UnTaza)도 함께 들어 있습니다.
- **한국어 파일 자동 찾기**: ELF와 같은 폴더에 둔 `LNG/`를 바로 읽습니다. 원래 OPL은 메모리카드의 `OPL/`이나 장치 루트의 `LNG/`만 찾습니다.
- **Gen2 게임 표지**: `ART/<게임 ID>_COV.png`뿐 아니라 `.jpg`, `.jpeg`, 그리고 `_COV`가 없는 파일 이름도 읽습니다. MMCE 기본 폴더를 설정했으면 그 폴더와 카드 루트의 `ART/`를 모두 찾습니다.
- **디스크 그림**: `ART/<게임 ID>_ICO.png`가 있을 때만 표시하고, 없으면 빈 CD 그림을 그리지 않습니다.
- **표지 읽기 안정성**: 잘린 PNG와 흑백 PNG를 안전하게 처리하고, 메모리가 모자랄 때 표지 캐시가 꼬이지 않도록 고쳤습니다.
- **게임별 메모리카드**: 게임을 실행할 때 게임 ID를 Gen2에 보내므로, Gen2가 게임마다 전용 메모리카드로 바꿉니다. 원래 MMCE OPL에 있는 기능이며, 이 빌드는 설정과 관계없이 항상 보냅니다.
- **PS1 게임**: 원래 OPL과 똑같이 동작합니다. 바꾼 것이 없습니다.

## 준비물

- FMCB 등으로 ELF를 실행할 수 있는 PS2
- Bitfunx PSxMemCard Gen2와 [sd2psXtd 1.4.0](https://github.com/sd2psXtd/firmware/releases/tag/1.4.0) 펌웨어
- Gen2 microSD에 넣은 PS2 게임 ISO. DVD 게임은 `DVD/`, CD 게임은 `CD/` 폴더에 넣습니다. MMCE 기본 폴더를 설정했다면 그 폴더 아래에 둡니다.

## 설치

### 1. 파일 복사

릴리스의 zip을 풀면 다음 파일이 나옵니다.

| 파일 | 설명 |
| --- | --- |
| `JJome-OPL.ELF` | OPL 본체 |
| `LNG/lang_Korean.lng` | 한국어 문구 |
| `LNG/font_Korean.ttf` | 한국어 글꼴 (UnTaza) |
| `LNG/UnTaza-COPYING.txt` | 글꼴 라이선스 |
| `bitfunx-mmce-ko.md` | 간단한 설치 안내 |
| `SHA256SUMS.txt` | 파일 확인용 SHA-256 |

`JJome-OPL.ELF`와 `LNG` 폴더를 **같은 폴더에 그대로** 복사합니다. 기존 OPL을 덮어쓰지 말고 따로 두세요.

- **FMCB 첫 화면에서 실행하려면** 메모리카드에 둡니다. 예: `mc0:/APPS/JJome-OPL/`. FMCB는 Gen2 microSD(`mmce0:`)에 있는 파일을 실행하지 못합니다. wLaunchELF로 메모리카드에 복사하면 됩니다.
- **메모리카드 공간이 부족하면** `LNG` 폴더만 Gen2 microSD 루트의 `LNG/`에 넣어도 됩니다. 이때는 MMCE 게임 목록이 열린 뒤에 언어 목록에 Korean이 나타납니다.
- **wLaunchELF로만 실행한다면** Gen2 microSD의 `APPS/JJome-OPL/`에 둬도 됩니다.

### 2. OPL 설정

1. JJome OPL을 실행합니다.
2. **설정 → MMCE 설정**에서 **MMCE 시작 모드**를 자동으로 바꿉니다.
3. **설정 → 화면 설정 → 언어**에서 `Korean`을 고릅니다.
4. 설정을 저장합니다.

PS1 게임이나 다른 앱을 OPL에서 실행하려면 **응용프로그램 시작 모드**도 켭니다.

### 3. 표지와 디스크 그림

Gen2 microSD의 `ART/` 폴더에 게임 ID 이름으로 넣습니다. 게임 ID는 `SLUS_123.45`처럼 ISO 이름 앞부분이나 OPL 게임 정보에서 확인할 수 있습니다.

| 그림 | 파일 이름 (찾는 순서) |
| --- | --- |
| 표지 | `<게임 ID>_COV.png` → `.jpg` → `.jpeg` → `<게임 ID>.png` → `.jpg` → `.jpeg` |
| 디스크 | `<게임 ID>_ICO.png` (없으면 표시하지 않음) |
| 그 밖의 그림 | 원래 OPL과 같음 (`_BG`, `_SCR` 등) |

MMCE 기본 폴더를 설정했다면 그 폴더의 `ART/`를 먼저 보고, 없으면 카드 루트의 `ART/`를 봅니다. 디스크 그림 변경은 OPL 기본 테마에만 적용됩니다. `THM/`의 다른 테마를 쓰면 그 테마 설정을 따릅니다.

### 4. FMCB 첫 화면에 등록

FMCB 설정 프로그램(`mc0:/SYS-CONF/FMCB_CFG.ELF`)을 실행합니다.

1. **Configure OSDSYS options...** → **Configure Item**에서 빈 항목을 고릅니다.
2. **Name**에 `JJome OPL`, **Path1**에 `mc0:/APPS/JJome-OPL/JJome-OPL.ELF`를 넣습니다.
3. 돌아가서 **Save CNF to MC0**로 저장합니다.

`mc0:/SYS-CONF/FREEMCB.CNF`를 직접 고쳐도 됩니다. `N`은 쓰지 않는 번호로 바꾸세요.

```ini
name_OSDSYS_ITEM_N = JJome OPL
path1_OSDSYS_ITEM_N = mc0:/APPS/JJome-OPL/JJome-OPL.ELF
```

## 한국어 파일을 찾는 위치

OPL은 `lang_Korean.lng`와 `font_Korean.ttf`를 다음 위치에서 찾습니다. 두 파일은 항상 같은 폴더에 있어야 합니다.

1. ELF가 있는 폴더의 `LNG/`, 또는 ELF가 있는 폴더 자체
2. 메모리카드의 `OPL/` 폴더 (`mc0:/OPL/lang_Korean.lng`)
3. Gen2 microSD 루트의 `LNG/`, MMCE 기본 폴더를 설정했다면 그 폴더의 `LNG/`
4. USB 등 BDM 장치 루트의 `LNG/`

같은 언어 파일이 여러 곳에 있으면 먼저 찾은 파일을 씁니다. 메모리카드를 가장 먼저 찾고, Gen2와 USB는 게임 목록을 읽을 때 찾습니다. 다른 버전의 `lang_Korean.lng`가 메모리카드 `OPL/`에 남아 있으면 지우세요. 문구 순서가 달라 메뉴 글자가 엉뚱하게 나옵니다.

## 게임 중 저장 공간이 부족할 때

Gen2는 게임마다 전용 메모리카드를 쓰므로 보통은 부족하지 않습니다. 부족하다는 메시지가 나오면 게임을 끄지 말고 저장 화면에서 다음처럼 합니다.

1. Gen2의 **오른쪽 버튼을 짧게** 눌러 다음 채널로 바꿉니다. 왼쪽 버튼을 짧게 누르면 이전 채널입니다. 채널마다 별도의 메모리카드이며 기본 8개입니다.
2. 게임 저장 화면에서 메모리카드를 다시 고른 뒤 저장합니다.
3. 저장한 채널 번호를 기억해 둡니다. 나중에 불러올 때 그 채널로 바꿔야 합니다.

저장하는 도중에는 버튼을 누르지 마세요. 카드가 바뀐 것을 인식하지 못하는 게임이라면 2번 슬롯에 일반 메모리카드를 꽂고 그쪽에 저장합니다. 버튼 동작은 [sd2psx 사용법](https://sd2psx.net/navigation.html)을 참고하세요.

## PS1 게임

PS1 처리는 원래 OPL과 같습니다. USB의 `POPS/`에 `POPSTARTER.ELF`를 `XX.게임명.ELF`로 복사해 두는 기존 방식을 그대로 씁니다.

- 게임 중 리셋(IGR)을 누르면 OPL이 아니라 `mc0:/BOOT/BOOT.ELF`(보통 wLaunchELF)로 나갑니다. POPStarter가 정하는 동작이라 OPL에서 바꿀 수 없습니다.
- 일부 게임이 버벅이는 것은 대개 POPStarter 호환성 문제입니다. 자세한 설정은 [POPStarter USB 문서](https://bitbucket.org/ShaolinAssassin/popstarter-documentation-stuff/wiki/usb-mode)를 참고하세요.
- PS1 게임을 표지와 함께 목록으로 고르고 싶다면 [POPSLoader](https://github.com/NathanNeurotic/POPSLoader)를 쓸 수 있습니다. 게임마다 `XX.*.ELF`를 만들 필요가 없습니다.

## 알려진 문제와 확인 상태

| 항목 | 상태 |
| --- | --- |
| 한국어 메뉴 | 실기 확인 |
| Gen2 표지 표시 | 실기 확인 |
| 디스크 그림 숨김 (테스트 빌드 6) | 실기 확인 전 |
| PS1 IGR이 wLaunchELF로 나감 | POPStarter 동작, 수정 불가 |
| 일부 PS1 게임 버벅임 | POPStarter 호환성 문제로 추정 |

## 직접 빌드하기

Linux 또는 WSL에 [PS2SDK](https://github.com/ps2dev/ps2sdk), [gsKit](https://github.com/ps2dev/gsKit), PS2SDK Ports(libpng, libjpeg)를 설치한 뒤 다음 순서로 빌드합니다.

```bash
make download_lwNBD
make src/lang_internal.c include/lang_autogen.h
make opl.elf
```

한국어 파일은 번역 저장소를 받은 뒤 만듭니다. `lng_tmpl/Korean-MMCE.yml`의 문구가 기존 번역 위에 덧씌워집니다.

```bash
make download_lng
make lng/lang_Korean.lng
```

배포용 ELF는 [PS2-Packer](https://github.com/ps2dev/ps2-packer)로 압축합니다.

## 원본과 바뀐 점

[ps2-mmce/Open-PS2-Loader](https://github.com/ps2-mmce/Open-PS2-Loader)의 `mmce_support` 브랜치(`e554796`, 2025-11-24)를 바탕으로 합니다. OPL MMCE Beta 2 이후의 공식 OPL 수정(GCC 15 대응, MX4SIO 회귀 수정, VMC 그룹)과 MMCE 알람·대기 사이클 설정 제거가 들어 있습니다. 여기에 다음을 더했습니다.

- 최신 PS2SDK의 `iopfixup` 요구에 맞춘 공식 OPL 수정 [`656cb1f`](https://github.com/ps2homebrew/Open-PS2-Loader/commit/656cb1f7)
- 공개 PS2SDK와 mmceman에 없는 `mmcesio2.irx` 로드를 빼고 Beta 2처럼 `mmceman.irx`만 로드
- 위 [주요 기능](#주요-기능)의 표지, 한국어, 디스크 그림 변경

주요 변경 파일은 `src/lang.c`, `src/mmcesupport.c`, `src/textures.c`, `src/texcache.c`, `misc/conf_theme_OPL.cfg`, `lng_tmpl/Korean-MMCE.yml`입니다.

### 브랜치

| 브랜치 | 내용 |
| --- | --- |
| `main` | 최신 코드와 이 설명서 |
| `bitfunx-mmce-next` | 개발 브랜치 |
| `bitfunx-mmce` | 초기 테스트 코드. Gen2에서 PS1을 실행하려던 시도가 들어 있으며 지금은 쓰지 않습니다. |

### `gen2 펌웨어/sd2psx.uf2`

[sd2psXtd 1.4.0 공식 배포본](https://github.com/sd2psXtd/firmware/releases/tag/1.4.0)과 SHA-256이 같은 펌웨어입니다. 만든이가 사용하는 버전을 함께 보관한 것이며, 최신 펌웨어와 소스는 원 저장소에서 받으세요.

## 출처와 라이선스

- OPL은 [Academic Free License 3.0](LICENSE)을 따릅니다. 원본 OPL 설명은 [README-OPL.md](README-OPL.md)에 있습니다.
- 한국어 번역은 [ddinghoya의 기존 OPL 번역](https://github.com/ps2homebrew/Open-PS2-Loader-lang/blob/b23b743d9f8c65e7a305db945f442e5755019221/Korean.yml)에 [MMCE 문구](lng_tmpl/Korean-MMCE.yml)를 더했습니다.
- `font_Korean.ttf`는 [OPL 번역 저장소에서 배포하는 UnTaza](https://github.com/ps2homebrew/Open-PS2-Loader-lang/blob/b23b743d9f8c65e7a305db945f442e5755019221/thirdparty/font_Korean.ttf)입니다. [UnTaza 글꼴 소스와 라이선스](https://ctan.org/tex-archive/fonts/unfonts-extra)를 참고하세요.
- MMCE 지원은 [ps2-mmce](https://github.com/ps2-mmce) 프로젝트의 작업입니다.
