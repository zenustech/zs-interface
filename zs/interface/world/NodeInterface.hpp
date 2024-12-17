#pragma once
#include "NodeDescriptor.hpp"
#include "ObjectInterface.hpp"
#include "interface/InterfaceExport.hpp"
#include "zensim/ZpcReflection.hpp"

namespace zs {

  using BasicFlagType = unsigned long long;
  using ResultType = unsigned long long;

  static_assert(sizeof(BasicFlagType) == 8 * sizeof(char), "...");
  static_assert(sizeof(ResultType) == 8 * sizeof(char), "...");

  enum Result : ResultType { Success = 0, Fail, Timeout, NumResultTypes };

  ///
  /// node (implemented by plugin developer)
  /// @note both creation (new etc.) and destruction (deinit) are within plugin
  /// realm
  ///
  struct ZS_INTERFACE_EXPORT NodeConcept {
    virtual ~NodeConcept() = default;

    static ZsList get_inputs_desc() { return {}; }
    static ZsList get_outputs_desc() { return {}; }
    static ZsDict get_attrib_desc() { return {}; }
    static ZsList get_category_desc() { return {}; }
    static ZsVar get_ui_desc() { return {}; }

    virtual ResultType setInput(const char *tag, ZsValue obj) { return Result::Fail; }
    virtual ZsValue getOutput(const char *tag) { return nullptr; }

    virtual ResultType preApply() { return Result::Success; }
    virtual ResultType apply() = 0;
    virtual ResultType postApply() { return Result::Success; }

    /// destructor implemented in user-defined node, where constructor
    virtual void deinit() { ::delete this; }  // called in Node's smart ptr deleter
  };

  ZS_INTERFACE_EXPORT ZsValuePort zs_build_node_ui_desc(NodeDescriptorPort port);

  ///
  /// context (implemented by the server)
  ///
  struct ZS_INTERFACE_EXPORT ContextConcept {
    virtual ~ContextConcept() = default;

    virtual ResultType createNode(ZsValue id, NodeConcept *node) { return Result::Fail; }
    virtual ResultType deleteNode(ZsValue id) { return Result::Fail; }
    virtual ResultType perform(ZsValue id) { return Result::Fail; }
    /// srcPin/dstPin is assumed list (for pin location)
    virtual ResultType createLink(ZsValue srcId, ZsValue srcPin, ZsValue dstId, ZsValue dstPin) {
      return Result::Fail;
    }
    virtual ResultType deleteLink(ZsValue srcId, ZsValue srcPin, ZsValue dstId, ZsValue dstPin) {
      return Result::Fail;
    }
    /// descriptor is assumed a dict
    virtual ResultType createPin(ZsValue id, ZsValue pin, ZsValue descriptor) {
      return Result::Fail;
    }
    virtual ResultType deletePin(ZsValue id, ZsValue pin) { return Result::Fail; }

    virtual void deinit() { ::delete this; }  // called in Node's smart ptr deleter
  };

  /// node manager (implemented by the server)
  using funcsig_create_node = NodeConcept *(ContextConcept *);
  using funcsig_get_node_ui_desc = ZsVar();
  struct ZS_INTERFACE_EXPORT NodeManagerConcept {
    virtual bool loadPluginsAt(const char *path) = 0;
    virtual bool registerNodeFactory(const char *label, funcsig_create_node *nodeFactory) = 0;
    virtual bool registerUiDescriptor(const char *label, funcsig_get_node_ui_desc *uiDesc) = 0;
    virtual bool registerNodeFactory(const char *label, funcsig_create_node *nodeFactory,
                                     NodeDescriptorPort descr)
        = 0;

    virtual funcsig_create_node *retrieveNodeFactory(const char *label) = 0;

    virtual void displayNodeFactories() = 0;
  };

  ///
  /// arranged interfaces
  ///

  /// @note recommend users inherit from this helper class
  template <typename Derived> struct NodeInterface {
    // factory method
    static NodeConcept *create_node(ContextConcept *) { return new Derived; }
  };

  // using funcsig_register_plugin = int(zs::NodeManagerConcept *, int *, int *);
  using funcsig_register_plugins = int(zs::NodeManagerConcept *, int *, int *);

#if 0
#  define ZS_REGISTER_PLUGIN_NODE(NODE_CLASS_NAME)                                               \
    extern "C" int register_node_factory(zs::NodeManagerConcept *manager, int *n, int *nTotal) { \
      bool done = true;                                                                          \
      done &= manager->registerNodeFactory(/*label*/ #NODE_CLASS_NAME,                           \
                                           /*method*/ NODE_CLASS_NAME::create_node);             \
      if (n) *n = done;                                                                          \
      if (nTotal) *nTotal = 1;                                                                   \
      return 0;                                                                                  \
    }
#endif

  template <typename T> static int compute_type_prefix_length() {
    constexpr auto typeTag = get_type_str<T>();
    if (typeTag[0] == 'c' && typeTag[1] == 'l' && typeTag[2] == 'a' && typeTag[3] == 's'
        && typeTag[4] == 's' && typeTag[5] == ' ')
      return 6;
    else if (typeTag[0] == 's' && typeTag[1] == 't' && typeTag[2] == 'r' && typeTag[3] == 'u'
             && typeTag[4] == 'c' && typeTag[5] == 't' && typeTag[6] == ' ')
      return 7;
    return 0;
  }
  template <typename... Ts>
  static int register_node_factories_helper(zs::NodeManagerConcept *manager, int *n, int *nTotal) {
    int n_ = 0;
    ((void)(n_
            += (manager->registerNodeFactory(
                    /*label*/ (const char *)get_type_str<Ts>() + compute_type_prefix_length<Ts>(),
                    /*method*/ Ts::create_node)
                && manager->registerUiDescriptor(
                    /*label*/ (const char *)get_type_str<Ts>() + compute_type_prefix_length<Ts>(),
                    /*method*/ Ts::get_ui_desc))),
     ...);
    if (n) *n = n_;
    if (nTotal) *nTotal = sizeof...(Ts);
    return 0;
  }
#define ZS_REGISTER_PLUGIN_NODES(...)                                                       \
  extern "C" ZS_EXPORT int register_node_factories(zs::NodeManagerConcept *manager, int *n, \
                                                   int *nTotal) {                           \
    return zs::register_node_factories_helper<__VA_ARGS__>(manager, n, nTotal);             \
  }

#define ZS_REGISTER_PLUGIN_NODE(NODE_CLASS_NAME) ZS_REGISTER_PLUGIN_NODES(NODE_CLASS_NAME)

}  // namespace zs