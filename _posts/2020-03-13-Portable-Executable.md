---
layout: post
title:  "What is EXE? (Introduction to Portable Executable)"
date:   2020-03-13
comments: true
categories: operating system 
tag:
- process
- windows
---

# What is EXE? (Introduction to Portable Executable)

## Introduction

EXE는 PE파일의  한 종류로 EXE파일을 실행시키면 windows의 PE Loader가 EXE를 loading한다. 즉 EXE파일에는 PE Loader가 어떻게 process를 실행시킬지에 대한  명세가 들어있다. 이 글에서는 EXE 정확히는 PE파일의 구조에 대해서 각각의 요소가 어떤 것을 의미하는 지에 대해서 써보고자 한다.

PE파일은 윈도우 운영 체제에서 사용되는 파일의 형식으로
