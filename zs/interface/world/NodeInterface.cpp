#include "NodeInterface.hpp"
#include "value_type/ValueInterface.hpp"

namespace zs {

ZsValuePort zs_build_node_ui_desc(NodeDescriptorPort desc) {
  ZsDict dict = zs_dict_obj_default();
  auto setDictItem = [](ZsDict dict, const char *tag, const char *item) {
    if (item && item[0] != '\0') {
      dict.setSteal(tag, zs_bytes_obj_cstr(item));
    }
  };
  // inputs
  ZsList inputList = zs_list_obj_default();
  for (unsigned i = 0; i < desc._numInputs; ++i) {
    ZsDict dictItem = zs_dict_obj_default();
    const auto &socketDesc = desc._inputDescriptors[i];
    setDictItem(dictItem, "type", socketDesc.type);
    setDictItem(dictItem, "name", socketDesc.name);
    setDictItem(dictItem, "defl", socketDesc.defl);
    setDictItem(dictItem, "doc", socketDesc.doc);
    inputList.appendSteal(dictItem);
  }
  // outputs
  ZsList outputList = zs_list_obj_default();
  for (unsigned i = 0; i < desc._numOutputs; ++i) {
    ZsDict dictItem = zs_dict_obj_default();
    const auto &socketDesc = desc._outputDescriptors[i];
    setDictItem(dictItem, "type", socketDesc.type);
    setDictItem(dictItem, "name", socketDesc.name);
    setDictItem(dictItem, "defl", socketDesc.defl);
    setDictItem(dictItem, "doc", socketDesc.doc);
    outputList.appendSteal(dictItem);
  }
  // attribs
  ZsList attribList = zs_list_obj_default();
  for (unsigned i = 0; i < desc._numAttribs; ++i) {
    ZsDict dictItem = zs_dict_obj_default();
    const auto &attribDesc = desc._attribDescriptors[i];
    setDictItem(dictItem, "type", attribDesc.type);
    setDictItem(dictItem, "name", attribDesc.name);
    setDictItem(dictItem, "defl", attribDesc.defl);
    setDictItem(dictItem, "doc", attribDesc.doc);
    attribList.appendSteal(dictItem);
  }
  // category
  ZsList categoryList = zs_list_obj_default();
  const char *st = desc._categoryDescriptor.category;
  const char *ed = st;
  if (ed) {
    while (*ed != '\0') ed++;
    const char *it = st;
    while (it != ed + 1) {
      // [st, it)
      while (*it != '\0' && *it != '.' && *it != '/') it++;
      categoryList.appendSteal(zs_bytes_obj_cstr_range(st, it - st));
      st = ++it;
    }
  }
  dict.setSteal("inputs", inputList);
  dict.setSteal("outputs", outputList);
  dict.setSteal("attribs", attribList);
  dict.setSteal("category", categoryList);
  return dict;
}

}