#pragma once

#include <functional>
#include <future>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>
#include <bitset>

namespace acpp {

namespace detail {

template <typename Callable, typename = typename std::enable_if_t<std::is_nothrow_invocable_v<Callable>>>
class scope_guard final {
public:
  static_assert( std::is_nothrow_invocable_v<Callable> );

  using callable_type = Callable;

  scope_guard( scope_guard &&other ) noexcept(
      std::is_nothrow_constructible_v<callable_type, callable_type &&> ) :
      m_callable{std::forward<callable_type>( other.m_callable )},
      m_is_valid{std::move( other.m_is_valid )} {
    other.m_is_valid = false;
  }

  ~scope_guard() noexcept {
    if ( m_is_valid ) {
      std::invoke( m_callable );
    }
  }

  void dismiss() noexcept { m_is_valid = false; }

public:
  scope_guard() = delete;
  scope_guard( const scope_guard & ) = delete;
  scope_guard &operator=( const scope_guard & ) = delete;
  scope_guard &operator=( scope_guard && ) = delete;

  static scope_guard make( callable_type &&callable ) noexcept(
      std::is_nothrow_constructible<callable_type, callable_type &&>::value ) {
    return scope_guard<callable_type>{std::forward<callable_type>( callable )};
  }

private:
  explicit scope_guard( Callable &&callable ) noexcept(
      std::is_nothrow_constructible<Callable, Callable &&>::value ) :
      m_callable{std::forward<Callable>( callable )},
      m_is_valid{true} {}

private:
  callable_type m_callable;
  bool m_is_valid = false;
};

}  // namespace detail

/**
 * Make a scope_guarded with a particular callable
 */
template <typename Callable>
inline constexpr detail::scope_guard<Callable> guard_scope( Callable &&callable ) noexcept(
    std::is_nothrow_constructible<Callable, Callable &&>::value ) {
  return detail::scope_guard<Callable>::make( std::forward<Callable>( callable ) );
}

/**
 * Void result - convenient alternative to exception
 * @tparam S std::error_code
 */
template <class S = std::error_code>
struct VResult {
  using value_type = void;
  using error_type = S;

  /**
   * @brief Construct a new VResult object in error condition
   * @param err
   * @param msg
   */
  VResult( const error_type &err, const std::string &msg ) noexcept :
      is_success( false ),
      err( err ),
      msg( msg ) {}

  VResult() noexcept = default;

  const bool is_success = true;
  const error_type err;
  const std::string msg;
};

/**
 * @brief Void result - convenient alternative to exception
 *
 * @tparam S std::error_code
 */
template <class R,
    class S = std::error_code,
    typename = typename std::enable_if<std::is_constructible_v<R>>::type>
struct Result : public VResult<S> {
  using value_type = R;
  using error_type = S;

  static_assert( std::is_constructible_v<value_type> );

  Result() = delete;

  Result( const error_type &err, const std::string &msg ) noexcept : VResult<error_type>( err, msg ) {}

  explicit Result(
      std::enable_if_t<std::is_copy_constructible_v<value_type>, const value_type &> value ) noexcept :
      value{value} {
    static_assert( std::is_copy_constructible_v<value_type> );
  }

  explicit Result( value_type &&value ) noexcept : value{std::forward<value_type>( value )} {}

  const value_type value;
};

/**
 * @brief Thin wrapper around bitset that accepts only enumeration values
 */
template <typename EnumT>
struct flagset {
  static_assert( std::is_enum<EnumT>::value && std::is_unsigned<typename std::underlying_type<EnumT>::type>::value );

  using flag_type = EnumT;
  using base_type = typename std::underlying_type<flag_type>::type;
  
  flagset() noexcept = default;
  flagset( const flagset &other ) noexcept = default;
  flagset & operator=( const flagset &other ) noexcept = default;
  flagset( flagset &&other ) noexcept = default;
  flagset &operator=( flagset &&other ) noexcept = default;

  constexpr flagset( const flag_type &flag ) noexcept : bitset{ static_cast<base_type>( flag ) } {
  }

  template <typename... Flags,
      typename = typename std::enable_if_t<std::conjunction_v<std::is_same<Flags, flag_type>...> > >
  constexpr flagset( const flag_type &flag1, const Flags&... flags ) noexcept :
      bitset{ static_cast<base_type>( flag1 ) | (static_cast<base_type>( flags ) | ...) } {
  }

  flagset &operator|=( const flag_type &flag ) {
    bitset |= static_cast<base_type>( flag );
    return *this;
  }
  flagset &operator&=( const flag_type &flag ) {
    bitset &= static_cast<base_type>( flag );
    return *this;
  }
  flagset &operator|=( const flagset &other ) {
    bitset |= other.bitset;
    return *this;
  }
  flagset &operator&=( const flagset &other ) {
    bitset &= other.bitset;
    return *this;
  }
  flagset &operator=( const flag_type &flag ) {
    bitset = static_cast<base_type>( flag );
    return *this;
  }
  flagset &operator+=( const flag_type &flag ) {
    bitset += static_cast<base_type>( flag );
    return *this;
  }
  flagset &operator-=( const flag_type &flag ) {
    bitset -= static_cast<base_type>( flag );
    return *this;
  }
  flagset operator&( const flag_type &flag ) const noexcept {
    return flagset { bitset & static_cast<base_type>( flag ) };
  }
  flagset operator&( const flagset &other ) const noexcept {
    return flagset { bitset & other.bitset };
  }
  flagset operator|( const flag_type &flag ) const noexcept {
    return flagset { bitset | static_cast<base_type>( flag ) };
  }
  flagset operator|( const flagset &other ) const noexcept {
    return flagset { bitset | other.bitset };
  }
  flagset operator~() const noexcept {
    return flagset { ~bitset };
  }
  explicit operator bool() const noexcept {
    return bitset.any();
  }
  explicit operator unsigned long() const noexcept {
    return bitset.to_ulong();
  }
  bool operator==( const flagset &other ) const noexcept {
    return bitset == other.bitset;
  }
  bool operator!=( const flagset &other ) const noexcept {
    return bitset != other.bitset;
  }
  constexpr std::size_t size() const noexcept {
    return bitset.size();
  }
  std::size_t count() const noexcept {
    return bitset.count();
  }
  flagset &set() {
    bitset.set();
    return *this;
  }
  flagset &reset() {
    bitset.reset();
    return *this;
  }
  flagset &flip() {
    bitset.flip();
    return *this;
  }
  flagset &set( const flag_type &flag, bool state = true ) {
    bitset.set( static_cast<base_type>( flag ), state );
    return *this;
  }
  flagset &reset( const flag_type &flag ) {
    bitset.reset( static_cast<base_type>( flag ) );
    return *this;
  }
  flagset &flip( const flag_type &flag ) {
    bitset.flip( static_cast<base_type>( flag ) );
    return *this;
  }
  constexpr bool operator[]( const flag_type &flag ) const {
    return bitset[static_cast<base_type>( flag )];
  }

  friend std::ostream &operator<<( std::ostream &stream, const flagset &flags ) {
    return stream << flags.bitset;
  }

private:
  std::bitset<static_cast<base_type>( flag_type::COUNT__ )> bitset;
  constexpr explicit flagset( const decltype( bitset ) &bitset ) : bitset{ bitset } {}

public:
  flagset( const std::string &repr ) : bitset( repr ) {}
  const auto &get_bitset() const { return bitset; }
};

template <typename EnumT>
using is_unsigned_scoped_enum = std::integral_constant<bool,
      std::is_enum<EnumT>::value &&
      std::is_unsigned<typename std::underlying_type<EnumT>::type>::value &&
      !std::is_convertible<EnumT, typename std::underlying_type<EnumT>::type>::value>;

}  // namespace acpp

/**
 * This works because scoped enumerations don't normally OR without static_cast
 */
template <typename EnumT>
constexpr typename std::enable_if<acpp::is_unsigned_scoped_enum<EnumT>::value, acpp::flagset<EnumT>>::type
    operator|( const EnumT &lhs, const EnumT &rhs ) {
  acpp::flagset<EnumT> first{lhs};
  return first | acpp::flagset<EnumT>{rhs};
}

inline void fstest() {
  enum class Flags : uint32_t { ONE, TWO, THREE, COUNT__ };
  //acpp::flagset<Flags> fs( Flags::ONE | Flags::TWO );
  acpp::flagset<Flags> fs2( Flags::ONE, Flags::TWO );
  //std::cout << fs2 << std::endl;
}
