---
layout: post
title: "데이터 중심 애플리케이션 설계 01"
date: 2023-12-12
comments: true
project: true
excerpt: "Designing Data-Intensive Applications"
categories: web
---

## Reliable, Scalable, and Maintainable Applications

Data-intensive application은 주로 요구되는 기능을 가지고 만들어진다. 예를 들면

-   데이터를 저장하고, 애플리케이션이나 다른 애플리케이션이 이를 나중에 찾을 수 있다. (database)
-   비싼 operation을 기억하고 있다가 빠르게 제공해 준다. (cache)
-   user에게 키워드 혹은 필터를 통해 데이터를 검색. (search indexes)
-   다른 process에게 async하게 처리되는 메세지를 보냄. (stream processing)
-   쌓인 대량의 데이터를 주기적으로 처리. (batch processing)
