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
pa_ext_device_restore_test.c
pa_ext_device_restore_test

PA_COMMAND_EXTENSION
SUBCOMMAND_TEST

pacmd unload-module module-udev-detect && pacmd load-module module-udev-detect
pacmd unload-module module-alsa-card && pacmd load-module module-alsa-card
locale 바꾸고 reload해야됨
tag에 넣고 그거 받아서 locale 설정
locale 설정뒤 module reload
