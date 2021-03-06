#include <string>
#include <iostream>
#include "IntermCode.hh"
#include "symbol.hh"
#include "statement.hh"
#include "program.hh"

extern Program program;
extern IntermCode intCode;
extern SymFunction* currentfun;

/*******************************/
/* Metodos de la clase Symbol */
/*****************************/

Symbol::Symbol(std::string id,int line,int col){
  this->id=id;
  this->line= line;
  this->col=col;
}

std::string Symbol::getId(){
  return this->id;
}

void Symbol::setDuplicated(bool dup){
  this->duplicated=dup;
}

bool Symbol::isDuplicated(){
  return this->duplicated;
}

void Symbol::setType(Type* t) {
  this->type = t;
}

Type* Symbol::getType(){
  return this->type;
}

int Symbol:: getnumScope(){
  return this->numScope;
}

int Symbol::getLine() {
  return this->line;
}

int Symbol::getColumn() {
  return this->col;
}


/***********************************/
/* Metodos de la clase SymFunction*/
/**********************************/

SymFunction::SymFunction(std::string id, ArgList* arguments, Type* ret,
			 int line, int col) : Symbol(id,line,col) {
  this->args = arguments;
  this->setType(ret); //tipo del retorno de la función

  this->start = new Label(std::string("f_")+id);
  this->generated = false;
}

void SymFunction::setBlock(Block* block) {
  this->block = block;
}

Block* SymFunction::getBlock(){
  return this->block;
}

int SymFunction::getArgumentCount(){
  return this->args->size();
}

void SymFunction::print() {
  std::cout << "Función " << id << " (" << line << ":" << col << ")" << std::endl;
  std::cout << "  Tipo: ";
  type->print();
  std::cout << std::endl;
  if (args->empty()) {
    std::cout << "  Sin argumentos" << std::endl;
  } else {
    std::cout << "  Argumentos:" << std::endl;
    for (ArgList::iterator it = args->begin();
	 it != args->end(); it++) {
      std::cout << "    Argumento: ";
      (*it)->print();
    }
  }
  // Imprimir bloque con nivel de anidamiento 1
  block->print(1);
}

ArgList* SymFunction::getArguments() {
  return this->args;
}

void SymFunction::check() {
  // Chequear que no se haya usado un tipo ilegal
  BoxType* bt = dynamic_cast<BoxType*>(this->getType());
  ArrayType* arrt = dynamic_cast<ArrayType*>(this->getType());
  StringType* strt = dynamic_cast<StringType*>(this->getType());
  if (bt or arrt or strt) {
    program.error("las funciones solo pueden devolver tipos básicos o void",
		  this->line, this->col);
    this->type = &(ErrorType::getInstance());
  }

  this->block->check();

  if (*(this->type) != VoidType::getInstance()
      && *(this->type) != ErrorType::getInstance()
      && !this->block->hasReturn()) {
    program.error("la función no tiene return", this->line, this->col);
  }

  int offset=0;
  for(ArgList::iterator it= this->args->begin(); it!=args->end(); it++){
    int align =(*it)->getAlignment();
    // Alinear offset de ser necesario, esperemos que nunca llegue aquí algo
    // que se tenga alignment 0 o super divisiones por 0 ocurrirán
    offset += (align - (offset % align)) % align;
    (*it)->setOffset(offset);
    offset += (*it)->getSize();
  }
  this->args_space = offset;
}

int SymFunction::getArgsSpace() {
  return this->args_space;
}

Label* SymFunction::getLabel() {
  return this->start;
}

Label* SymFunction::getEpilogueLabel() {
  return this->epilogue;
}

void SymFunction::addReturnTarget(BasicBlock* b) {
  this->ret_targets.push_back(b);
}

std::list<BasicBlock*> SymFunction::getReturnTargets() {
  return this->ret_targets;
}

void SymFunction::setLocalSpace(int space) {
  this->local_space = space;
}

int SymFunction::getLocalSpace() {
  return local_space;
}

/**********************************/
/*** Metodos de la clase SymVar ***/
/**********************************/

SymVar::SymVar(std::string id,int line,int col,
               bool isParam, int scope) : Symbol(id,line,col){
  this->numScope = scope;
  this->isParameter= isParam;
  this->readonly = false;
  this->reference = false;
  //  std::cout << "SymVar creado";
  this->temp = false;
  this->mem = true;
}

SymVar::SymVar(int idTemp) {
  this->id = ".t"+std::to_string((long long int) idTemp);
  this->idTemp = idTemp;
  this->temp = true;
  this->reference=false;
  this->mem = false;
  this->offset = -1;
  this->isParameter = false;
}


void SymVar::print(){
  std::cout << this->type->toString() << " " << id << " (" << line << ":" << col
	    << ") [Bloque: " << this->context
	    << "] [Offset: " << this->offset << "]" << std::endl;
}

void SymVar::setReadonly(bool readonly){
  // Pasar por readonly implica pasarlo por referencia
  this->readonly = readonly;
  this->reference = readonly;
}

void SymVar::setReference(bool ref) {
  // Pasar por referencia implica que NO se pasa readonly
  this->reference = ref;
  this->readonly = false;
}

bool SymVar::isReadonly(){
  return this->readonly;
}

bool SymVar::isReference() {
  return this->reference;
}

void SymVar::setContext(int num) {
  this->context = num;
}

void SymVar::setOffset(int offset){
  this->offset= offset;
}

int SymVar::getSize(){
  if (isParameter and reference)
    return this->type->getReferenceSize();
  return this->type->getSize();
}

int SymVar::getAlignment() {
  if (isParameter and reference)
    return 8;
  return this->type->getAlignment();
}

int SymVar::getOffset(){
  return this->spill();
}

void SymVar::setType(Type* type) {
  this->type = type;
  ArrayType* arrt = dynamic_cast<ArrayType*>(type);
  if (arrt && isParameter && arrt->getLength() > 0) {
    program.error("No se puede especificar un tamaño de arreglo en "
		  "la declaración de una función", this->line, this->col);
    return;
  }
  if (arrt && !isParameter && arrt->getLength() == 0) {
    program.error("debe especificar un tamaño válido para el arreglo",
		  this->line, this->col);
    return;
  }
}

bool SymVar::isTemp(){
  return this->temp;
}

void SymVar::inMem(bool m) {
  this->mem = m;
}

bool SymVar::isInMem() {
  return this->mem;
}

void SymVar::addReg(Reg r) {
  //  std::cout << "# Colocando " << id << " en " << regToString(r) << std::endl;
  this->locations.insert(r);
}

void SymVar::removeReg(Reg r) {
  //  std::cout << "# Borrando " << id << " en " << regToString(r) << std::endl;
  this->locations.erase(r);
}

bool SymVar::availableOther(Reg r) {
  if (mem) return true;
  std::set<Reg> l = this->locations; // Teoricamente es una copia
  l.erase(r);
  return !l.empty();
}

bool SymVar::availableReg() {
  return this->locations.size() > 0;
}

bool SymVar::isGlobal() {
  return this->context == 1;
}

void SymVar::setLabel(Label* l) {
  this->glabel = l;
}

Label* SymVar::getLabel() {
  return this->glabel;
}

int SymVar::spill() {
  if (!temp or offset > -1) {
    if (isParameter) {
      return offset;
    } else {
      return -(offset+12);
    }
  }
  
  this->offset = currentfun->getLocalSpace();
  currentfun->setLocalSpace(this->offset+4);
  std::cout << "# Spill de " << id << ", offset " << offset << std::endl;
  return -(offset+12);
}

std::set<Reg> SymVar::getAllLocations() {
  return this->locations;
}

bool SymVar::isInReg(Reg r) {
  return locations.count(r) > 0;
}

// WARN
Reg SymVar::getLocation() {
  // Devuelve un registro donde se encuentre esta variable
  if (locations.begin() != locations.end()) {
    return *(locations.begin());
  }
  return Reg::zero;
}

/************************************/
/*** Metodos de la clase SymTable ***/
/************************************/

SymTable::SymTable(){
  this->nextscope=2;
  this->stack.push_back(0);
  this->stack.push_back(1);
}

int SymTable::current_scope(){
  return this->stack[stack.size()-1];
}

void SymTable::insert(SymVar *sym){
  //std::cout << "insertando var "<<sym->getId()<<std::endl;
  sym->setContext(this->current_scope());
  this->varTable.insert(varSymtable::value_type(sym->getId(),sym));

}

void SymTable::insert(SymFunction *sym){
  //  std::cout << "insertando fun "<<sym->getId()<<std::endl;
  this->funcTable.insert(funcSymtable::value_type(sym->getId(),sym));
}

void SymTable::insert(BoxType *sym){
  this->boxTable.insert(boxHash::value_type(sym->getName(),sym));
}

int SymTable::leave_scope(){
  this->stack.pop_back();
}

int SymTable::enter_scope(){
  this->stack.push_back((this->nextscope)++);
}


SymFunction* SymTable::lookup_function(std::string nombreID){
  funcSymtable::iterator it= this->funcTable.find(nombreID);

  if (it!= funcTable.end())
    return it->second;
  else
    return NULL;
}

BoxType* SymTable::lookup_box(std::string nombreID){
  boxHash::iterator it= this->boxTable.find(nombreID);

  if (it!= boxTable.end())
    return it->second;
  else
    return NULL;
}

SymVar* SymTable::lookup_variable(std::string nombreID){
  /*Buscar en cualquier contexto*/
  SymVar *best=NULL;
  SymVar *pervasive=NULL;

  std::pair<varSymtable::iterator, varSymtable::iterator> pair1;
  pair1= this->varTable.equal_range(nombreID);

  /*Recorrer todos los nombres encontrados. Tomado del complemento
    del capitulo 3 del Scott*/
  for (; pair1.first != pair1.second; ++pair1.first){
    if (pair1.first->second->getnumScope()==0)
      pervasive= pair1.first->second;
    else{
      int i=this->stack.size()-1;
      for(i;i>0;i--){
        if (this->stack[i]==pair1.first->second->getnumScope()){
          best=pair1.first->second;
          break;
        }else if(best!=NULL && this->stack[i]==best->getnumScope())
          break;
      }
    }
  }

  if (best!=NULL)
    return best;
  else if (pervasive!=NULL)
    return pervasive;
  else
    return NULL;

}

void SymTable::print(){

  std::cout << std::endl << "**** Tabla de simbolos ****" << std::endl<< std::endl;
  std::cout << "Variables:" << std::endl;

  for(varSymtable::iterator it= this->varTable.begin();
      it!= this->varTable.end(); it++){
    (*it).second->print();
  }

}
