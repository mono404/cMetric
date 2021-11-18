# cMetric
resource monitoring in k8s

Check your gcc version before build the project

g++ cMetics.cpp -o cMetrics -std=c++11

This source code requires Metrics Server to be correctly configured and working on the server.

# Resource Monitoring in k8s
## 해당 프로젝트 선택 이유
 - 개발팀과 배포/운영팀이 다르다보니 소프트웨어에 문제가 생길 시 피드백이 원활하지 않음
 - 개발팀은 배포과정을 모르기 때문에 디버깅을 수행하기 어려움
 - 쿠버네티스 내의 어떠한 node, node내의 어떠한 pod, pod내의 어떠한 프로세스에 문제가 생겼는지 빠르게 파악하기 위해

## 프로젝트 개요
> 쿠버네티스 환경 내의 리소스 모니터링 프로그램
- 쿠버네티스 클러스터 기본 구조 파악
  - Nodes
    - Namespaces
        - Pods
            - Precesses
- 쿠버네티스 기본 Workfolw 이해
- 쿠버네티스 확장 컴포넌트 사용
  - Metrics Server API
- CRI-O 사용
  - crictl

## 프로젝트 진행 시 활용한 툴과 이론에 대한 설명, 본인의 역할
> 쿠버네티스 클러스터에 대한 이해
- 쿠버네티스 API
  - Command-Line API
  - REST API
- 쿠버네티스 컴포넌트
  - cAdvisor
    - Metrics Server
  - CRI-O
    - circtl
> 기존 모니터링 도구 단점 개선
- 기존 모니터링 도구는 namespcae에 대한, Pod 내 프로세스에 대한 리소스 모니터링을 지원 안함

## 코드 분석
- C/C++ 기반
- getopt_long을 통해 GNU, POSIX 스타일의 옵션 지원
- Node, Pod에 대한 리소스는 Metrics Server API 사용
- Namespace, Process에 대한 리소스는 CRI-O API 사용

## 구현
> 실행전 Metrics Server 설치 필요
- 빌드 방법
  - g++ cMetics.cpp -o cMetrics -std=c++11
- 노드에 대한 리소스 정보
  - ./cMetrics get nodes
  - ./cMetrics get node [NODE_NAME]
- 네임 스페이스에 대한 리소스 정보
  - ./cMetric.out get namespaces
  - ./cMetric.out get namespace [NAMESAPCE_NAME]
- 파드에 대한 리소스 정보
  - ./cMetric.out get pods
  - ./cMetric.out get pods --all-namespaces
  - ./cMetric.out get pods --namespace=[NAMESPACE_NAME]
  - ./cMetric.out get pods –N [NAMESPACE_NAME]
- 프로세스에 대한 리소스 정보
  - ./cMetric.out get pods [POD_NAME]
  - ./cMetric.out get pods [POD_NAME] --namespace=[NAMESPACE_NAME]

## 프로젝트에서 배운 점
- 쿠버네티스 기본기
- 다양한 API 사용법, 활용법
- 실무에서의 쿠버네티스 경험

## 프로젝트의 보완점 및 개선안
- 옵션에 따른 다양한 정보 제공
- GUI 적용
- 커맨드라인 API -> REST API
- 웹 모니터링 툴 제공
