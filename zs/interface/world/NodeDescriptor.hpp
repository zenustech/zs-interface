#pragma once

namespace zs {

using PChar = const char *;

struct PlaceHolder {};
///
/// attrib
///
struct AttribDescriptor {
  const char *type = "", *name = "", *defl = "", *doc = "";

  constexpr AttribDescriptor() noexcept = default;
  constexpr AttribDescriptor(const char *name) noexcept
      : type{""}, name{name}, defl{""}, doc{""} {}
  constexpr AttribDescriptor(const char *type, const char *name) noexcept
      : type{type}, name{name}, defl{""}, doc{""} {}
  constexpr AttribDescriptor(const char *type, const char *name,
                             const char *defl) noexcept
      : type{type}, name{name}, defl{defl}, doc{""} {}
  constexpr AttribDescriptor(const char *type, const char *name,
                             const char *defl, const char *doc) noexcept
      : type{type}, name{name}, defl{defl}, doc{doc} {}
  constexpr AttribDescriptor(const PChar (&strs)[1]) noexcept
      : type{""}, name{strs[0]}, defl{""}, doc{""} {}
  constexpr AttribDescriptor(const PChar (&strs)[2]) noexcept
      : type{strs[0]}, name{strs[1]}, defl{""}, doc{""} {}
  constexpr AttribDescriptor(const PChar (&strs)[3]) noexcept
      : type{strs[0]}, name{strs[1]}, defl{strs[2]}, doc{""} {}
  constexpr AttribDescriptor(const PChar (&strs)[4]) noexcept
      : type{strs[0]}, name{strs[1]}, defl{strs[2]}, doc{strs[3]} {}
};

template <unsigned N> struct AttribDescriptors {
  static constexpr unsigned size = N;
  AttribDescriptor attribs[N];

  constexpr AttribDescriptors(const AttribDescriptors &) = default;
  constexpr AttribDescriptors(AttribDescriptors &&) noexcept = default;
  constexpr AttribDescriptors &operator=(const AttribDescriptors &) = default;
  constexpr AttribDescriptors &
  operator=(AttribDescriptors &&) noexcept = default;

  template <unsigned... ns>
  constexpr AttribDescriptors(const PChar (&...list)[ns]) : attribs{list...} {}
  constexpr AttribDescriptors(const AttribDescriptor (&list)[N]) : attribs{} {
    for (unsigned i = 0; i < N; ++i)
      attribs[i] = list[i];
  }
  constexpr AttribDescriptors(const AttribDescriptor &item) : attribs{item} {}
};
template <> struct AttribDescriptors<0> {
  static constexpr unsigned size = 0;
  AttribDescriptor *attribs = nullptr;
};

template <unsigned... Ns>
AttribDescriptors(const PChar (&...list)[Ns])
    -> AttribDescriptors<sizeof...(Ns)>;
template <unsigned N>
AttribDescriptors(const AttribDescriptor (&list)[N]) -> AttribDescriptors<N>;
AttribDescriptors(const AttribDescriptor &item)->AttribDescriptors<1>;
AttribDescriptors()->AttribDescriptors<0>;

///
/// category
///
struct CategoryDescriptor {
  const char *category;

  constexpr CategoryDescriptor(const char *category = "uncategorized") noexcept
      : category{category} {}
  template <unsigned... Ns>
  constexpr CategoryDescriptor(const char (&...strs)[Ns]) noexcept
      : category{} {
    const char *vs[] = {strs...};
    category = vs[0];
  }
  template <unsigned M>
  constexpr CategoryDescriptor(const PChar (&strs)[M]) noexcept
      : category{strs[0]} {}
};

///
/// socket
///
struct SocketDescriptor {
  const char *type = "", *name = "", *defl = "", *doc = "";

  constexpr SocketDescriptor() noexcept = default;
  constexpr SocketDescriptor(const char *name) noexcept
      : type{""}, name{name}, defl{""}, doc{""} {}
  constexpr SocketDescriptor(const char *type, const char *name) noexcept
      : type{type}, name{name}, defl{""}, doc{""} {}
  constexpr SocketDescriptor(const char *type, const char *name,
                             const char *defl) noexcept
      : type{type}, name{name}, defl{defl}, doc{""} {}
  constexpr SocketDescriptor(const char *type, const char *name,
                             const char *defl, const char *doc) noexcept
      : type{type}, name{name}, defl{defl}, doc{doc} {}
  constexpr SocketDescriptor(const PChar (&strs)[1]) noexcept
      : type{""}, name{strs[0]}, defl{""}, doc{""} {}
  constexpr SocketDescriptor(const PChar (&strs)[2]) noexcept
      : type{strs[0]}, name{strs[1]}, defl{""}, doc{""} {}
  constexpr SocketDescriptor(const PChar (&strs)[3]) noexcept
      : type{strs[0]}, name{strs[1]}, defl{strs[2]}, doc{""} {}
  constexpr SocketDescriptor(const PChar (&strs)[4]) noexcept
      : type{strs[0]}, name{strs[1]}, defl{strs[2]}, doc{strs[3]} {}
};

template <unsigned N> struct SocketDescriptors {
  static constexpr unsigned size = N;
  SocketDescriptor sockets[N];

  constexpr SocketDescriptors(const SocketDescriptors &) = default;
  constexpr SocketDescriptors(SocketDescriptors &&) noexcept = default;
  constexpr SocketDescriptors &operator=(const SocketDescriptors &) = default;
  constexpr SocketDescriptors &
  operator=(SocketDescriptors &&) noexcept = default;

  template <unsigned... ns>
  constexpr SocketDescriptors(const PChar (&...list)[ns]) : sockets{list...} {}
  constexpr SocketDescriptors(const SocketDescriptor (&list)[N]) : sockets{} {
    for (unsigned i = 0; i < N; ++i)
      sockets[i] = list[i];
  }
  constexpr SocketDescriptors(const SocketDescriptor &item) : sockets{item} {}
};
template <> struct SocketDescriptors<0> {
  static constexpr unsigned size = 0;
  SocketDescriptor *sockets = nullptr;
};

template <unsigned... Ns>
SocketDescriptors(const PChar (&...list)[Ns])
    -> SocketDescriptors<sizeof...(Ns)>;
template <unsigned N>
SocketDescriptors(const SocketDescriptor (&list)[N]) -> SocketDescriptors<N>;
SocketDescriptors(const SocketDescriptor &item)->SocketDescriptors<1>;
SocketDescriptors()->SocketDescriptors<0>;

struct NodeDescriptorPort {
  const SocketDescriptor *_inputDescriptors{nullptr};
  unsigned _numInputs{0};
  const SocketDescriptor *_outputDescriptors{nullptr};
  unsigned _numOutputs{0};
  const AttribDescriptor *_attribDescriptors{nullptr};
  unsigned _numAttribs{0};
  CategoryDescriptor _categoryDescriptor{};
};
template <unsigned NumInputs, unsigned NumOutputs, unsigned NumAttribs>
struct Descriptor {
  SocketDescriptors<NumInputs> _inputs;
  SocketDescriptors<NumOutputs> _outputs;
  AttribDescriptors<NumAttribs> _attribs;
  CategoryDescriptor _category;

  NodeDescriptorPort getView() const noexcept {
    return NodeDescriptorPort{
        _inputs.sockets,  _inputs.size,  _outputs.sockets, _outputs.size,
        _attribs.attribs, _attribs.size, _category};
  }

  constexpr Descriptor(SocketDescriptors<NumInputs> inputs,
                       SocketDescriptors<NumOutputs> outputs,
                       AttribDescriptors<NumAttribs> attribs,
                       CategoryDescriptor category) noexcept
      : _inputs{inputs}, _outputs{outputs}, _attribs{attribs}, _category{
                                                                   category} {}
  template <unsigned NInputs, unsigned NOutputs, unsigned NAttribs>
  constexpr Descriptor(const SocketDescriptor (&inputs)[NInputs],
                       const SocketDescriptor (&outputs)[NOutputs],
                       const AttribDescriptor (&attribs)[NAttribs],
                       CategoryDescriptor category = {})
      : _inputs{inputs}, _outputs{outputs}, _attribs{attribs}, _category{
                                                                   category} {}

  template <unsigned NOutputs, unsigned NAttribs>
  constexpr Descriptor(PlaceHolder, const SocketDescriptor (&outputs)[NOutputs],
                       const AttribDescriptor (&attribs)[NAttribs],
                       CategoryDescriptor category = {})
      : _inputs{}, _outputs{outputs}, _attribs{attribs}, _category{category} {}
  template <unsigned NInputs, unsigned NAttribs>
  constexpr Descriptor(const SocketDescriptor (&inputs)[NInputs], PlaceHolder,
                       const AttribDescriptor (&attribs)[NAttribs],
                       CategoryDescriptor category = {})
      : _inputs{inputs}, _outputs{}, _attribs{attribs}, _category{category} {}
  template <unsigned NInputs, unsigned NOutputs>
  constexpr Descriptor(const SocketDescriptor (&inputs)[NInputs],
                       const SocketDescriptor (&outputs)[NOutputs], PlaceHolder,
                       CategoryDescriptor category = {})
      : _inputs{inputs}, _outputs{outputs}, _attribs{}, _category{category} {}

  template <unsigned NAttribs>
  constexpr Descriptor(PlaceHolder, PlaceHolder,
                       const AttribDescriptor (&attribs)[NAttribs],
                       CategoryDescriptor category = {})
      : _inputs{}, _outputs{}, _attribs{attribs}, _category{category} {}
  template <unsigned NOutputs>
  constexpr Descriptor(PlaceHolder, const SocketDescriptor (&outputs)[NOutputs],
                       PlaceHolder, CategoryDescriptor category = {})
      : _inputs{}, _outputs{outputs}, _attribs{}, _category{category} {}
  template <unsigned NInputs>
  constexpr Descriptor(const SocketDescriptor (&inputs)[NInputs], PlaceHolder,
                       PlaceHolder, CategoryDescriptor category = {})
      : _inputs{inputs}, _outputs{}, _attribs{}, _category{category} {}

  template <unsigned NCategory>
  constexpr Descriptor(PlaceHolder, PlaceHolder, PlaceHolder,
                       CategoryDescriptor category = {})
      : _inputs{}, _outputs{}, _attribs{}, _category{category} {}
};

template <unsigned NumInputs, unsigned NumOutputs, unsigned NumAttribs>
Descriptor(const SocketDescriptor (&a)[NumInputs],
           const SocketDescriptor (&b)[NumOutputs],
           const AttribDescriptor (&c)[NumAttribs], const CategoryDescriptor &)
    -> Descriptor<NumInputs, NumOutputs, NumAttribs>;

template <unsigned NumOutputs, unsigned NumAttribs>
Descriptor(PlaceHolder, const SocketDescriptor (&b)[NumOutputs],
           const AttribDescriptor (&c)[NumAttribs], const CategoryDescriptor &)
    -> Descriptor<0, NumOutputs, NumAttribs>;
template <unsigned NumInputs, unsigned NumAttribs>
Descriptor(const SocketDescriptor (&a)[NumInputs], PlaceHolder,
           const AttribDescriptor (&c)[NumAttribs], const CategoryDescriptor &)
    -> Descriptor<NumInputs, 0, NumAttribs>;
template <unsigned NumInputs, unsigned NumOutputs>
Descriptor(const SocketDescriptor (&a)[NumInputs],
           const SocketDescriptor (&b)[NumOutputs], PlaceHolder,
           const CategoryDescriptor &) -> Descriptor<NumInputs, NumOutputs, 0>;

template <unsigned NumAttribs>
Descriptor(PlaceHolder, PlaceHolder, const AttribDescriptor (&c)[NumAttribs],
           const CategoryDescriptor &) -> Descriptor<0, 0, NumAttribs>;
template <unsigned NumOutputs>
Descriptor(PlaceHolder, const SocketDescriptor (&b)[NumOutputs], PlaceHolder,
           const CategoryDescriptor &) -> Descriptor<0, NumOutputs, 0>;
template <unsigned NumInputs>
Descriptor(const SocketDescriptor (&a)[NumInputs], PlaceHolder, PlaceHolder,
           const CategoryDescriptor &) -> Descriptor<NumInputs, 0, 0>;

Descriptor(PlaceHolder, PlaceHolder, PlaceHolder, const CategoryDescriptor &)
    ->Descriptor<0, 0, 0>;

} // namespace zs