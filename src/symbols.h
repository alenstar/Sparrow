#ifndef SPARROW_SYMBOLS_H_
#define SPARROW_SYMBOLS_H_

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <exception>

/*符号表：
 * 在遍历AST时，保存常量在常量池的位置信息
 * 记录局部变量在函数运行时环境的位置信息
 */

/**************************符号表异常******************************/
class SymbolsException: public std::exception {
public:
  SymbolsException(const std::string &msg): msg_(msg) {}
  const char* what() const noexcept override {
    return msg_.c_str();
  }
private:
  std::string msg_;
};

/**************************常量符号表******************************/
template <typename T>
class ConstantSymbols {
public:
  ConstantSymbols() = default;

  //获取常量在常量池的地址，如果不存在则放入其中
  size_t getIndex(T constant) {
    if (symbolsIndex_.find(constant) != symbolsIndex_.end()) {
      return symbolsIndex_[constant];
    }
    else {
      constPool_.push_back(constant);
      size_t result = constPool_.size() - 1;
      symbolsIndex_[constant] = result;
      return result;
    }
  }

  //获取常量池中的变量
  T get(size_t index) {
    if (index >= constPool_.size())
      throw SymbolsException("const value access out of range");
    return constPool_[index];
  }
  
  //获取常量池
  std::vector<T> &getConstantPool() const {
    return constPool_;
  }

private:
  //常量下标
  std::map<T, size_t> symbolsIndex_;

  //常量池
  std::vector<T> constPool_;
};
using IntSymbolsPtr = std::shared_ptr<ConstantSymbols<int>>;
extern IntSymbolsPtr g_IntSymbols;

using FloatSymbolsPtr = std::shared_ptr<ConstantSymbols<double>>;
extern FloatSymbolsPtr g_FloatSymbols;

using StrSymbolsPtr = std::shared_ptr<ConstantSymbols<std::string>>;
extern StrSymbolsPtr g_StrSymbols;


/**************************通用符号表******************************/
enum class SymbolsKind {
  UNKNOWN, UNIT, CLASS, FUNCTION
};

class Symbols;
using SymbolsPtr = std::shared_ptr<Symbols>;

class Symbols {
public:
  Symbols(SymbolsPtr outer, SymbolsKind kind);

  //获取局部变量在函数运行时环境的位置
  //如果该变量为全局变量或类变量，则返回-1
  //如果该变量为闭包上层函数的临时变量，返回（-2 - 下标），结果值 + 2可反向逆推
  int getRuntimeIndex(const std::string &name);

  //当前传入变量被强制当作临时变量处理
  //用于函数形参预处理，防止形参和全局变量一致
  //由于类的成员变量总是当作全局变量预处理，所以不需要调用该函数
  int forceGetLocalIndex(const std::string &name);

  //获取符号表大小
  size_t getSymbolSize() const;

  //添加某个类的符号表
  //只有当当前符号表时unit的符号表时才有效
  void putClassSymbols(const std::string &className, SymbolsPtr symbols);

  //获取某个类的符号表
  //只有当当前符号表是unit的符号表时才有效
  SymbolsPtr getClassSymbols(const std::string &className); 

private:
  //定位符号所在的符号表，如果找不到返回空指针
  SymbolsPtr locateSymbol(SymbolsPtr symbol, const std::string &name);
  
private:
  //符号表类型
  SymbolsKind kind_ = SymbolsKind::UNKNOWN;

  //上一层符号表
  SymbolsPtr outer_;

  //变量以及它保存的位置，即使是全局变量也会分配位置（这些位置信息是没用的）
  std::map<std::string, size_t> symbolsIndex_;

  //类符号表<类名，符号表>
  //只有当当前符号表是unit的符号表时才会使用该项
  std::map<std::string, SymbolsPtr> classSymbols_;
};

#endif
