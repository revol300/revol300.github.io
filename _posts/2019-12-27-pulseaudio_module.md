---
layout: post
title:  "pulseaudio에 추가기능을 넣으려면 어떻게 해야할까??"
date:   2019-12-27
comments: true
excerpt: "Writing new pulseaudio modules"
categories: pulseaudio 
tag:
- c
- multimedia
- pulseaudio
---
# Writing new PulseAudio modules 

[Writing new PulseAudio modules](https://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/Developer/Modules/)의 내용을 일부 번역하였습니다.

## Introduction

PulseAudio는 오픈소스로 직접 소스코드를 보면서 그 동작원리를 이해할 수 있다. 하지만, 현실적으로 내가 소스 코드만을 가지고  PulseAudio를 이해하기에는 알고 있는 바가 너무 없다. 다행히 [PulseAudio under the hood](https://gavv.github.io/articles/pulseaudio-under-the-hood/)라는 좋은 글과 [PulseAudio Documentation](https://freedesktop.org/software/pulseaudio/doxygen/) 을 통해 PulseAudio의 간단한 사용과 Pulseaudio 이해의 첫 발은 내딛었다. 이제 더 깊은 이해를 위해 [PulseAudio Developer Page](https://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/Developer/)의 글들을 하나하나 읽어가면서 정리하고자 한다. 이 글의 순서대로 PulseAudio의 module을 어떻게 작성하는지에 대해 내용을 먼저 요약하려 한다.

## What is a module?

module은 shared object file로 특정 함수를 구현해놓았다. daemon은 이 module을 미리 정의된 디렉토리(기본값 : "/usr/local/lib/pulse-/modules/")에서 읽어온다. module의 파일 이름은 "module-"로 시작하며 시스템 별로 다른 확장자를 가진다(ex: .so). 파일 이름은 daemon의 시작 configuration 파일 또는 pactl 프로그램에서 "load-module"명령에 identifier로 사용된다. 예를 들어 "load-module module-sine"은 데몬에게 module-sine.so 파일에서 모듈을 로드하라는 의미이다.

## Compiling to a shared object file

아래와 같이 .so 파일을 만들고 이를 module directory 에 옮기면 pactl 에서 "load-module module-"를 실행함으로써 module을 쉽게 load할 수 있다.
```c
gcc -g -shared -o module-<yourmodule>.so module-<yourmodule>.c
```
하지만 다른 파일이나 라이브러리의 정의와 변수들을 사용하는 경우도 있기 때문에 위와 같이 간단한 일은 아니다. PulseAudio의 configure.ac 및 Makefile.am 을 업데이트 하면 모듈을 자동으로 올리는 것이 가능하다. 다음과 같이 configure.ac를 작성하면된다.

```autoconf
...
#### ABC support ####

AC_ARG_ENABLE([abc],
              AC_HEML_STRING([--disable-abc], [Disable optional ABC module support]),
              [
                case "${enableval}" in
                  yes) abc=yes ;;
                  no) abc=no ;;
                  *) AC_MSG_ERROR(bad value ${enableval} for --disable-abc) ;;
                esac
              ],
              [abc=no])

if test "x${abc}" != xno ; then
  if test "x$HAVE_DBUS" = x1 && test "x$HAVE_GLIB20" = x1 ; then
    AC_DEFINE([ABC], 1, [Have ABC module.])
    ABC=1
  else
    ABC=0
  fi
else
  ABC=0
fi
AM_CONDITIONAL([ABC], [test "x$ABC" = x1])
...
```

다음 코드는 abc module을 src/Makefile.am 에 추가 했을 때의 코드 이다

```autoconf
if ABC
  modlibexec_LTLIBRARIES += \
    libdbus-util.la \
    module-abc.la
endif
...
module_abc_la_SOURCES = modules/module-abc.c
module_abc_la_LDFLAGS = -module -avoid-version
module_abc_la_LIBADD = $(AM_LIBADD) $(DBUS_LIBS) $(GLIB20_LIBS) libpulsecore.la
module_abc_la_CFLAGS = $(AM_CFLAGS) $(DBUS_CFLAGS) $(GLIB20_CFLAGS)
...
```

위 코드에는 abc가 다른 라이브러리에 대해 dependency를 가지고 있기 때문에 위 코드를 알맞은 위치에 배치해야 함을 숙지하자.

## Required functions

daemon이 module을 load하기 위해서는 다음 함수를 무조건 구현해야 한다.
```c
int pa__init(pa_module* m);
```
pa__init 은 daemon이 module을 load한 시점에서 불린다. pa__init에서 되도록 module의 초기화를 해주도록 하자. daemon은 pa__init의 return 값을 통해 이 함수가 성공했는지 여부를 알아낸다. 음수는 실패를 의미하며 0 또는 양수는 성공을 의미한다.

몇몇 module은 initialize만 필요할 수도 있지만, 대부분의 module은  unload시 cleanup이 필요할 것이다. 이는 pa__done을 구현함으로써 이루어진다

```c
void pa__done(pa_module* m);
```

> pa_module type은module.h에 정의되어 있으며 이에 대해서는 후에 더 자세히 알아보도록 하자.

그래서 daemon이 module을 열려면 최소한 다음 과 같은 구색은 갖춰야한다.

#include <pulsecore/module.h>

int pa__init(pa_module* m){
  return 0;
}

## Optional functions

위 함수 이외에도 작성할 수 있는 몇몇 함수들이 더 있다.

```c
const char* pa__get_author();

const char* pa__get_description();

const char* pa__get_usage();

const char* pa__get_version();
```

이들은 module에 대한 추가적인 정보를 제공해주며 다음을 통해서 추가적인 정보를 알아낼 수 있다.

```bash
pulseaudio --dump-modules --verbose
```

위의 함수들을 macro를 써서 더 간결하게 표현할 수도 있다. module-null-sink.c를 예로 들면,

```c
PA_MODULE_AUTHOR("Lennart Pottering")
PA_MODULE_DESCRIPTION("Clocked NULL sink")
PA_MODULE_VERSION(PACKAGE_VERSION)
PA_MODULE_USAGE(
      "format=<sample format> "
      "channels=<number of channels> "
      "rate=<sample rate> "
      "sink_name=<name of sink>"
      "channel_map=<channel map>"
      "description=<description for the sink>")
```

## Static linking of modules
동적 로딩 뿐만이 아니라, daemon에서 모듈 정적 링크도 가능하다. 물론 모든 모듈이 자신의 pa__init를 정의하기 때문에 일반적으로는 하나의 모듈에 대해서만 정적 링크를 사용할 수 있다. 하지만, PulseAudio는 libtool을 사용하여 이러한 문제없이 동일한 코드를 동적으로 또는 정적으로 링크 할 수있는 방법을 제공한다. 

이를 위해서는 소스 파일에서 포함할 헤더 파일을 작성해야합니다. null-sink를 예로 들어보다. 다음은 module-null-sink.c가 포함할 module-null-sink-symdef.h의 내용이다.

```c
#ifndef foomodulenullsinksymdeffoo
#define foomodulenullsinksymdeffoo

#include <pulsecore/module.h>

#define pa__init module_null_sink_LTX_pa__init
#define pa__done module_null_sink_LTX_pa__done
#define pa__get_author module_null_sink_LTX_pa__get_author
#define pa__get_description module_null_sink_LTX_pa__get_description
#define pa__get_usage module_null_sink_LTX_pa__get_usage
#define pa__get_version module_null_sink_LTX_pa__get_version

int pa__init(pa_module*m);
void pa__done(pa_module*m);

const char* pa__get_author(void);
const char* pa__get_description(void);
const char* pa__get_usage(void);
const char* pa__get_version(void);

#endif
```

물론 이는 수동적인 방식으로 pulseaudio 개발자는 M4 scipt를 사용해서 build time에 이 symdef file을 생성할 수 있다. [source : branches / lennart / src / Makefile.am Makefile.am] ( "SYMDEF"검색) 및 [source : branches / lennart / src / modules / module-defs.h.m4 모듈 /module-defs.h .m4]를 참조하자.

## State data
pa__init 과 pa__done에서 module은 internal data를 어떻게 관리할까? 이는 pa_module과 연관이 있다. pa_module에는 사용자 데이터 필드가 있다. 이는 void 포인터 타입으로, 그 목적은 module의 internal data에 대한 포인터를 저장에 있다. 따라서 pa__init에서 데이터에 사용하려는 일부 데이터 구조를 할당한 다음 pa_module (매개 변수로 제공) userdata 필드를 통해 할당된 data를 사용 가능하다. pa__done에서도 마찬가지로 동일한 pa_module 매개 변수를 찾을 수 있으며 이를 통해 내부 데이터를 찾을 수 있다.

