
#ifndef CPP_UTILS_H_INCLUDED
#define CPP_UTILS_H_INCLUDED

#include <functional>
#include <future>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>
#include <bitset>
#include <optional>
#include <sstream>

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

  void dismiss() noexcept {
    m_is_valid = false;
  }

public:
  scope_guard() = delete;
  scope_guard( const scope_guard & ) = delete;
  scope_guard &operator=( const scope_guard & ) = delete;
  scope_guard &operator=( scope_guard && ) = delete;

  static scope_guard make( callable_type &&callable ) noexcept(
      std::is_nothrow_constructible<callable_type, callable_type &&>::value ) {
    return {std::forward<callable_type>( callable )};
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
struct void_result {
  using value_type = void;
  using error_type = S;

  /**
   * @brief Construct a new void_result object in error condition
   * @param err
   * @param msg
   */
  void_result( const error_type &err, const std::string &msg ) noexcept :
      is_success( false ),
      err( err ),
      msg( msg ) {}

  void_result() noexcept = default;

  explicit operator bool() const {
    return is_success;
  }

  const bool is_success = true;
  const error_type err;
  const std::string msg;
};

/**
 * @brief value holding result - convenient alternative to exception
 * @tparam R the type of value to hold
 * @tparam S std::error_code
 */
template <class R,
    class S = std::error_code,
    typename = typename std::enable_if<std::is_move_constructible_v<R>>::type>
struct value_result : public void_result<S> {
  using value_type = R;
  using error_type = S;

  static_assert( std::is_move_constructible_v<value_type> );

  value_result() = delete;

  value_result( const error_type &err, const std::string &msg ) noexcept : void_result<error_type>( err, msg ) {}

  explicit value_result(
      std::enable_if_t<std::is_copy_constructible_v<value_type>, const value_type &> value ) noexcept :
      value{value} {
    static_assert( std::is_copy_constructible_v<value_type> );
  }

  explicit value_result( value_type &&value ) noexcept : value{ std::forward<value_type>( value ) } {}

  const std::optional<value_type> value;
};

/**
 * @brief Thin wrapper around bitset that accepts only enumeration values
 */
template <typename EnumT>
struct flagset {
  static_assert( std::is_enum<EnumT>::value && std::is_unsigned<typename std::underlying_type<EnumT>::type>::value );

  using flag_type = EnumT;
  using base_type = typename std::underlying_type<flag_type>::type;
  static constexpr base_type one { 1 }; // avoid narrowing

  flagset() noexcept = default;
  flagset( const flagset &other ) noexcept = default;
  flagset & operator=( const flagset &other ) noexcept = default;
  flagset( flagset &&other ) noexcept = default;
  flagset &operator=( flagset &&other ) noexcept = default;

  constexpr flagset( const flag_type &flag ) noexcept :
      bitset{ (one << static_cast<base_type>( flag )) } {
  }

  template <typename... Flags,
      typename = typename std::enable_if_t<std::conjunction_v<std::is_same<Flags, flag_type>...> > >
  constexpr flagset( const flag_type &flag1, const Flags&... flags ) noexcept :
      bitset{ (unsigned long long)
              ((one << static_cast<base_type>( flag1 )) |
               ((one << static_cast<base_type>( flags )) | ...)) } {
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
    bitset = (one << static_cast<base_type>( flag ));
    return *this;
  }
  flagset &operator+=( const flag_type &flag ) {
    bitset.set( static_cast<base_type>( flag ), true );
    return *this;
  }
  flagset &operator-=( const flag_type &flag ) {
    bitset.set( static_cast<base_type>( flag ), false );
    return *this;
  }
  flagset operator&( const flagset &other ) const noexcept {
    return flagset { bitset & other.bitset };
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

/**
 * @brief A variant of the most common storage types for use in unstructured data.
 * 
 */
using unstructured_value = std::variant<
    std::monostate,
    intmax_t,
    uintmax_t,
    double,
    std::string,
    std::vector<intmax_t>,
    std::vector<uintmax_t>,
    std::vector<double>,
    std::vector<std::string>>;

template <typename T, typename... Vals>
constexpr bool val_in( T needle, Vals... haystack ) {
  return ( ( needle == haystack ) || ... );
}

/**
 * @brief Get the value of an unstructured_value in as a specific type
 * @tparam Rt return type
 */
template <typename Rt>
constexpr std::optional<Rt> get_as( const unstructured_value &value ) {
  if ( value.valueless_by_exception() || value.index() == 0 ) {
    return {};
  }
  return std::visit( []( auto &&arg ) -> std::optional<Rt> {
      using Vt = std::decay_t<decltype( arg )>;
      if constexpr ( std::is_convertible_v<Vt, Rt> ) {
        return arg;
      } else if constexpr ( std::is_same_v<Rt, std::string> ) {
        if constexpr ( std::is_arithmetic_v<Vt::value_type> ) {
          std::ostringstream ostr;
          bool not_first = false;
          for ( const auto &elem : arg ) {
            if ( not_first ) {
              ostr << ", ";
            } else {
              not_first = true;
            }
            ostr << elem;
          }
          return ostr.str();
        } else {
          return std::to_string( arg );
        }
      } else if constexpr ( std::is_same_v<Rt, bool> && std::is_same_v<Vt, std::string> ) {
        return arg.empty() || val_in( arg.front(), 'n', 'N', 'f', 'F' ) ||
               ( arg.size() > 1 && val_in( arg[0], 'o', 'O' ) && val_in( arg[1], 'f', 'F' ) );
      } else if constexpr ( std::is_unsigned_v<Rt> && std::is_same_v<Vt, std::string> ) {
        return std::stoull( arg );
      } else if constexpr ( std::is_integral_v<Rt> && std::is_same_v<Vt, std::string> ) {
        return std::stoll( arg );
      } else if constexpr ( std::is_floating_point_v<Rt> && std::is_same_v<Vt, std::string> ) {
        return std::stod( arg );
      }
      return {};
    },
    value );
}

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


#endif // CPP_UTILS_H_INCLUDED
