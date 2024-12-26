#pragma once
#include "interface/InterfaceExport.hpp"
#include "zensim/TypeAlias.hpp"

namespace zs {

  using CommandType = u32;
  enum command_type_e : CommandType { base_cmd_ = 0, py_cmd_ };

  struct ZS_INTERFACE_EXPORT CommandConcept {
    virtual ~CommandConcept() = default;

    virtual CommandType getType() const { return command_type_e::base_cmd_; }
    virtual void execute() = 0;
    virtual bool undo() { return false; }                      // not necessary
    virtual CommandConcept *clone() const { return nullptr; }  // useful for plugin
    virtual void deinit() { ::delete this; }
  };

  struct ZS_INTERFACE_EXPORT PyCommandConcept : CommandConcept {
    virtual ~PyCommandConcept() = default;

    CommandType getType() const override { return command_type_e::py_cmd_; }
  };

}  // namespace zs