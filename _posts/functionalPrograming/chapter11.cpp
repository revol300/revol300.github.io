// 템플릿 메타프로그래밍

// 컴파일 시점에 유형 조작

template <typename T>
using contained_type_t = decltype(*begin(T()));

std::declval <T>()


