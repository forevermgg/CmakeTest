#ifndef BASE_MACROS_H_
#define BASE_MACROS_H_

#define BASE_DISALLOW_COPY(TypeName) TypeName(const TypeName&) = delete

#define BASE_DISALLOW_ASSIGN(TypeName) \
  TypeName& operator=(const TypeName&) = delete

#define BASE_DISALLOW_MOVE(TypeName) \
  TypeName(TypeName&&) = delete;    \
  TypeName& operator=(TypeName&&) = delete

#define BASE_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;          \
  TypeName& operator=(const TypeName&) = delete

#define BASE_DISALLOW_COPY_ASSIGN_AND_MOVE(TypeName) \
  TypeName(const TypeName&) = delete;               \
  TypeName(TypeName&&) = delete;                    \
  TypeName& operator=(const TypeName&) = delete;    \
  TypeName& operator=(TypeName&&) = delete

#define BASE_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName() = delete;                               \
  BASE_DISALLOW_COPY_ASSIGN_AND_MOVE(TypeName)

#define BASE_FRIEND_TEST(test_case_name, test_name) \
  friend class test_case_name##_##test_name##_Test

#endif  // BASE_MACROS_H_
