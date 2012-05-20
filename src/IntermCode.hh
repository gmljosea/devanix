#include <list>
#include <unordered_map>
#include "quad.hh"


class Label;

typedef std::unordered_multimap<int,Label> labels;

class ItermCode {
private:
  // Conjunto de etiquetas asociadas a instrucciones
  labels labelset;
  // Conjunto de etiquetas sin asignar
  std::list<Label*> unSet;
  // Lista de instrucciones
  std::list<Quad*> inst;
  // Num prox etiqueta
  int nextlabel;

public:
  IntermCode ():nextlabel(0);
  Label* newLabel();
  Temp newTemp();
  void addInst(Quad* quad);
  void emitLabel(Label* label);
  void emitLabel2();
  bool areUnSet();
};

class Label {
private:
  Quad* instruction;
  int id;

public:
  Label(int id): id(id);
  void setInstruction(Quad* quad);
};

// Temp tendra varios constructores dependiendo de si es constant float int sym etc
