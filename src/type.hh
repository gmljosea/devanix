#ifndef DEVANIX_TYPES
#define DEVANIX_TYPES

#include <map>
#include <string>

class Type {
protected:
  int size;
  int alignment;
public:
  Type(int size, int alignment) : size(size), alignment(alignment) {};
  virtual int getSize();
  virtual int getAlignment();
  virtual bool operator==(Type& b);
};

// Tipos básicos escalares
class IntType : public Type {
private:
  IntType() : Type(4, 4) {};
  void operator=(IntType const&);
public:
  static IntType& getInstance();
};

class FloatType : public Type {
private:
  FloatType() : Type(4, 4) {};
  void operator=(FloatType const&);
public:
  static FloatType& getInstance();
};

class BoolType : public Type {
private:
  BoolType() : Type(1, 1) {};
  void operator=(BoolType const&);
public:
  static BoolType& getInstance();
};

class CharType : public Type {
private:
  CharType() : Type(1, 1) {};
  void operator=(CharType const&);
public:
  static CharType& getInstance();
};

// Tipos especiales
class VoidType : public Type {
private:
  VoidType() : Type(0, 0) {};
  void operator=(VoidType const&);
public:
  static VoidType& getInstance();
};

class StringType : public Type {
public:
  StringType(int length) : Type(length, 1) {};
};

class ErrorType : public Type {
private:
  ErrorType() : Type(0, 0) {};
  void operator=(ErrorType const&);
public:
  static ErrorType& getInstance();
};

// Tipos compuestos
class ArrayType : public Type {
private:
  Type* basetype;
  int length;
public:
  ArrayType(Type* btype, int length) : basetype(btype), length(length),
                                       Type(0,0) {};
  virtual bool operator==(Type& t);
  virtual int getSize();
  virtual int getAlignment();
  Type* getBaseType();
  int getLength();
  int getOffset(int pos); //offset de la posición pos
};

struct BoxField {
  Type* type;
  std::string name;
  int offset;
    bool braced;
};

class BoxType : public Type {
private:
  std::string name;
  std::map<std::string, BoxField*> fixed_fields;
  std::map<std::string, BoxField*> variant_fields;
  bool incomplete;
protected:
  bool reaches(BoxType& box);
public:
  BoxType(std::string name, bool incomplete)
    : name(name), incomplete(incomplete), Type(0,0) {};
  void addFixedField(Type* type, std::string name);
  void addVariantField(Type* type, std::string name, bool braced);
  BoxField* getField(std::string field);
  void check();
  bool isIncomplete();
  void setIncomplete(bool ic);
  std::string getName();
};

#endif
