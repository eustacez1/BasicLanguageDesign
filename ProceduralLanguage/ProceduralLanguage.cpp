#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
using namespace std;

/*****************************************************************
 *                     DECLARATIONS                              *
 *****************************************************************/
typedef int NUMBER;
typedef int NAME;
const int  NAMELENG = 20;      /* Maximum length of a name */
const int  MAXNAMES = 100;     /* Maximum number of different names */
const int  MAXINPUT = 5000;     /* Maximum length of an input */
const char*   PROMPT = "-> ";
const char*   PROMPT2 = "> ";
const char  COMMENTCHAR = ';';
const int   TABCODE = 9;        /* in ASCII */

struct EXPLISTREC;
typedef EXPLISTREC* EXPLIST;
enum EXPTYPE {VALEXP,VAREXP,APEXP};
struct EXPREC
{
      EXPTYPE etype; //what type of expression
      NUMBER num;
      NAME varble;
      NAME optr;
      EXPLIST args;
};
typedef EXPREC* EXP;

struct EXPLISTREC
{
      EXP head;
      EXPLIST tail;
};

struct VALUELISTREC
{
      NUMBER  head;
      VALUELISTREC*  tail;
};

typedef VALUELISTREC* VALUELIST;

struct NAMELISTREC
{
      NAME   head;
      NAMELISTREC* tail;
};
typedef NAMELISTREC* NAMELIST;

struct  ENVREC
{
       NAMELIST vars;
       VALUELIST values;
};

typedef ENVREC* ENV;

struct  FUNDEFREC
{
       NAME  funname;
       NAMELIST  formals;
       EXP  body;
       FUNDEFREC*  nextfundef;
};
typedef FUNDEFREC* FUNDEF;

FUNDEF  fundefs; //head to the global funtions

ENV globalEnv; //head to the global env

EXP currentExp;  // points to the expression that we want to compute
char userinput[MAXINPUT];
int   inputleng, pos;

char*   printNames[MAXNAMES];  //symbol table
int   numNames, numBuiltins;
int   quittingtime;

/*****************************************************************
 *                     DATA STRUCTURE OP'S                       *
 *****************************************************************/

/* mkVALEXP - return an EXP of type VALEXP with num n            */

EXP mkVALEXP ( NUMBER n)
{
   EXP e;
   e = new EXPREC;
   e->etype = VALEXP;
   e->num = n;
   return e;
}/* mkVALEXP */


/* mkVAREXP - return an EXP of type VAREXP with varble nm  */

EXP mkVAREXP ( NAME nm)
{
   EXP e;
   e = new EXPREC;
   e->etype = VAREXP;
   e->varble = nm;
   return e;
}/* mkVAREXP */


/* mkAPEXP - return EXP of type APEXP w/ optr op and args el     */

EXP mkAPEXP (NAME op, EXPLIST el)
{
   EXP  e;
   e = new EXPREC;
   e->etype = APEXP;
   e->optr = op;
   e->args = el;
   return e;
}/* mkAPEXP */

/* mkExplist - return an EXPLIST with head e and tail el         */

EXPLIST mkExplist (EXP e, EXPLIST el)
{
    EXPLIST newel;
    newel =new EXPLISTREC;
    newel->head = e;
    newel->tail = el;
    return newel;
}/* mkExplist */

/* mkNamelist - return a NAMELIST with head n and tail nl        */

NAMELIST mkNamelist ( NAME nm, NAMELIST nl)
{
   NAMELIST newnl;
   newnl = new NAMELISTREC;
   newnl->head = nm;
   newnl->tail = nl;
   return newnl;
}/* mkNamelist */

/* mkValuelist - return an VALUELIST with head n and tail vl     */

VALUELIST mkValuelist (NUMBER n,  VALUELIST vl)
{
   VALUELIST newvl;
   newvl = new VALUELISTREC;
   newvl->head = n;
   newvl->tail = vl;
   return newvl;
}/* mkValuelist */

/* mkEnv - return an ENV with vars nl and values vl              */

ENV mkEnv ( NAMELIST nl, VALUELIST vl)
{
    ENV rho;
    rho = new ENVREC;
    rho->vars = nl;
    rho->values = vl;
    return rho;
}/* mkEnv */

/* lengthVL - return length of VALUELIST vl      */

int lengthVL ( VALUELIST vl)
{
   int i = 0;
   while (vl != 0)
   {
     i++;
     vl = vl->tail;
   }
   return i;
}/* lengthVL */

/* lengthNL - return length of NAMELIST nl    */

int lengthNL ( NAMELIST nl)
{
   int i = 0;
   while( nl !=0 )
   {
     ++i;
     nl = nl->tail;
   }
   return i;
}/* lengthNL */

/*****************************************************************
 *                     NAME MANAGEMENT                           *
 *****************************************************************/

/* fetchFun - get function definition of fname from fundefs */

FUNDEF fetchFun ( NAME fname)
{
   FUNDEF  f;
   f = fundefs;
   while (f != 0)
   {
   
     if (f->funname == fname )
        return f;
     f = f->nextfundef;
    }
  return 0;
}/* fetchFun */


/* newFunDef - add new function fname w/ parameters nl, body e   */

void  newFunDef (NAME fname,  NAMELIST nl, EXP e)
{
   FUNDEF f;
   f = fetchFun(fname);
   if (f == 0) /* fname not yet defined as a function */
   {
     f = new FUNDEFREC;
     f->nextfundef = fundefs; // place new FUNDEFREC
     fundefs = f;        // on fundefs list
   }
   f->funname = fname;
   f->formals = nl;
   f->body = e;
}// newFunDef


/* initNames - place all pre-defined names into printNames */

void initNames()
{
   int i =0;
   fundefs = 0;
   printNames[i] = (char* )"if";      i++;
   printNames[i] = (char* )"while";   i++;
   printNames[i] = (char*)"set";      i++;
   printNames[i] = (char* )"begin";    i++;
   printNames[i] = (char* )"+";       i++;
   printNames[i] = (char* )"-";       i++;
   printNames[i] = (char* ) "*";       i++;
   printNames[i] = (char* )"/";       i++;
   printNames[i] = (char* )"=";       i++;
   printNames[i] = (char* )"<";       i++;
   printNames[i] = (char* )">";       i++;
   printNames[i] = (char* )"print";
   numNames = i;
   numBuiltins = i;
}//initNames

/* install - insert new name into printNames  */

NAME install ( char* nm)
{
   int i = 0;
   while (i <= numNames)
   {
         if (strcmp( nm,printNames[i] ) == 0)
          return i;
      i++;
   }
   if (i > numNames)
   {
      numNames = i;
      printNames[i] = new char[strlen(nm) + 1];
      strcpy(printNames[i], nm);
   }
   return i;
}// install

/* prName - print name nm              */

void prName ( NAME nm)
{
     cout<< printNames[nm];
} //prName

/*****************************************************************
 *                        INPUT                                  *
 *****************************************************************/

/* isDelim - check if c is a delimiter   */

int isDelim (char c)
{
   return ( ( c == '(') || ( c == ')') ||( c == ' ')||( c== COMMENTCHAR) );
}

/* skipblanks - return next non-blank position in userinput */

int skipblanks (int p)
{
   while (userinput[p] == ' ')
    ++p;
   return p;
}


/* matches - check if string nm matches userinput[s .. s+leng]   */

int matches (int s, int leng,  char* nm)
{
   int i=0;
   while (i < leng )
   {
     if( userinput[s] != nm[i] )
        return 0;
     ++i;
     ++s;
    }
   if (!isDelim(userinput[s]) )
      return 0;
   return 1;
}/* matches */



/* nextchar - read next char - filter tabs and comments */

void nextchar (char& c)
{
    scanf("%c", &c);
    if (c == COMMENTCHAR )
    {
      while ( c != '\n' )
        scanf("%c",&c);
    }
}


/* readParens - read char's, ignoring newlines, to matching ')' */
void readParens()
{
   int parencnt; /* current depth of parentheses */
   char c;
   parencnt = 1; // '(' just read
   do
   {
      if (c == '\n')
        cout <<PROMPT2;
      cout.flush();
      nextchar(c);
      pos++;
      if (pos == MAXINPUT )
      {
        cout <<"User input too long\n";
        exit(1);
      }
      if (c == '\n' )
        userinput[pos] = ' ';
      else
        userinput[pos] = c;
      if (c == '(')
        ++parencnt;
      if (c == ')')
        parencnt--;
    }
    while (parencnt != 0 );
} //readParens

/* readInput - read char's into userinput */

void readInput()
{
    char  c;
    cout << PROMPT;
    cout.flush();
    pos = 0;
    do
     {
        ++pos ;
        if (pos == MAXINPUT )
        {
            cout << "User input too long\n";
            exit(1);
        }
        nextchar(c);
        if (c == '\n' )
           userinput[pos] = ' ';
        else
           userinput[pos] = c;
        if (userinput[pos] == '(' )
          readParens();
     }
    while (c != '\n');
    inputleng = pos;
    userinput[pos+1] = COMMENTCHAR; // sentinel
}


/* reader - read char's into userinput; be sure input not blank  */

void reader ()
{
    do
    {
      readInput();
      pos = skipblanks(1);
    }
    while( pos > inputleng); // ignore blank lines
}

/* parseName - return (installed) NAME starting at userinput[pos]*/

NAME parseName()
{
   char nm[20]; // array to accumulate characters
   int leng; // length of name
   leng = 0;
   while ( (pos <= inputleng) && !isDelim(userinput[pos]) )
   {
        
        nm[leng] = userinput[pos];
        ++pos;
        ++leng;
   }
   if (leng == 0)
   {
       cout<<"Error: expected name, instead read: "<< userinput[pos]<<endl;
       exit(1);
   }
   nm[leng] = '\0';
   pos = skipblanks(pos); // skip blanks after name
   return ( install(nm) );
}// parseName

/* isDigits - check if sequence of digits begins at pos   */

int isDigits (int pos)
{
   if ( ( userinput[pos] < '0' ) ||  ( userinput[pos] > '9' ) )
      return 0;
   while ( ( userinput[pos] >='0') && ( userinput[pos] <= '9') )
     ++pos;
   if (!isDelim(userinput[pos] ))
     return 0;
   return 1;
}// isDigits


/* isNumber - check if a number begins at pos  */

int isNumber (int pos)
{
   return ( isDigits(pos) ||
             ( (userinput[pos] == '-') && isDigits(pos+1))
            ||( (userinput[pos] == '+') && isDigits(pos+1)));
}// isNumber

/* parseVal - return number starting at userinput[pos]   */

NUMBER parseVal()
{
   int  n=0, sign=1;
   pos = skipblanks(pos); // skip blanks
   if (userinput[pos] == '+')
      ++pos;
   if (userinput[pos] == '-')
   {
     sign = -1;
     ++pos;
   }
   while ( (userinput[pos] >= '0') && ( userinput[pos] <= '9') )
   {
        n = 10*n + userinput[pos] -'0';
        ++pos;
   }
   pos = skipblanks(pos); // skip blanks after number
   return ( (NUMBER) n*sign);
}// parseVal

EXPLIST parseEL();

/* parseExp - return EXP starting at userinput[pos]  */

EXP parseExp()
{
   NAME nm;
   EXPLIST el;
   pos = skipblanks(pos); // skip blanks
   if ( userinput[pos] == '(' )
   {// APEXP
      pos = skipblanks(pos+1); // skip '( ..'
      nm = parseName();
      el = parseEL();
      return ( mkAPEXP(nm, el));
   }
   if (isNumber(pos))
      return ( mkVALEXP((NUMBER)parseVal() ));  // VALEXP
   return ( mkVAREXP((NAME)parseName() ) ); // VAREXP
}// parseExp

/* parseEL - return EXPLIST starting at userinput[pos]  */

EXPLIST parseEL()
{
   EXP e;
   EXPLIST el;
   pos = skipblanks(pos); // skip blanks
   if ( userinput[pos] == ')')
   {
     pos = skipblanks(pos+1); // skip ') ..'
     return 0;
   }
   e = parseExp();
   el = parseEL();
   return ( mkExplist(e, el));
}// parseEL


/* parseNL - return NAMELIST starting at userinput[pos]  */

NAMELIST parseNL()
{
    NAME nm;
    NAMELIST nl;
   pos = skipblanks(pos); // skip blanks
   if ( userinput[pos] == ')' )
   {
      pos = skipblanks(pos+1); // skip ') ..'
      return 0;
   }
   nm = parseName();
   nl = parseNL();
   return ( mkNamelist(nm, nl));
}// parseNL

/* parseDef - parse function definition at userinput[pos]   */

NAME parseDef()
{
    NAME fname;        // function name
    NAMELIST nl;       // formal parameters
    EXP e;             // body
   pos = skipblanks(pos); // skip blanks
    pos = skipblanks(pos+1); // skip '( ..'
    pos = skipblanks(pos+6); // skip 'define ..'
    fname = parseName();
    pos = skipblanks(pos+1); // skip '( ..'
    nl = parseNL();
    e = parseExp();
   pos = skipblanks(pos); // skip blanks
    pos = skipblanks(pos+1); // skip ') ..'
    newFunDef(fname, nl, e);
   return ( fname);
}// parseDef


//some auxilary function that you need
ENV emptyEnv()
{
   return  mkEnv(0, 0);
}

/* bindVar - bind variable nm to value n in environment rho */

void bindVar ( NAME nm,  NUMBER n,  ENV rho)
{
  //fix this
  rho->vars = mkNamelist(nm,rho->vars);
  rho->values = mkValuelist(n,rho->values);
}

/* findVar - look up nm in rho   */


VALUELIST findVar ( NAME nm, ENV rho)
{
   // do this

   VALUELIST curVar = rho->values;
   NAMELIST curName = rho->vars;

   while (curVar != 0)
   {
      if( curName->head == nm) return curVar;

      curVar = curVar->tail;
      curName = curName->tail;
   }
   
   return 0;
}


/* assign - assign value n to variable nm in rho   */

void  assign (NAME nm,  NUMBER n, ENV rho)
{
   VALUELIST varloc;
   varloc = findVar(nm, rho);
   varloc->head = n;
}// assign

/* fetch - return number bound to nm in rho */

NUMBER fetch ( NAME nm, ENV rho)
{
   VALUELIST  vl;
   vl = findVar(nm, rho);
   return (vl->head);
}

/* isBound - check if nm is bound in rho  */

int isBound ( NAME nm, ENV rho)
{
   return ( findVar(nm, rho) != 0 );
}


/*****************************************************************
 *                     NUMBERS                                   *
 *****************************************************************/

/* prValue - print number n    */

void prValue ( NUMBER n )
{
   cout << n;
}
// prValue


/* isTrueVal - return true if n is a true (non-zero) value  */

int isTrueVal ( NUMBER n)
{
   return ( n != 0 );
}// isTrueVal



NUMBER applyValueOp ( int op,  VALUELIST vl)
{
   NUMBER  n, n1, n2;


   n1 = vl->head; // 1st actual
   if (op == 11)  {
          prValue(n1);
        cout<<endl;
        return n1;
   }

   // if ( arity(op) != lengthVL(vl) )// correct number of parameters
   // {
    //   cout <<    "Wrong number of arguments to  ";
    //   prName(op);
    //   cout <<endl;
   //  }

   n2 = vl->tail->head; //2nd actual
   switch (op) // do this
   {
     case 4 : n = n1+n2; break;
     case 5 : n = n1-n2; break;
     case 6 : n = n1*n2; break;
     case 7 : n = n1/n2; break;
     case 8 : n = n1==n2; break;
     case 9: n = n1<n2;  break;
     case 10: n = n1>n2; break;
     
   };//switch
   return n;
}// applyValueOp
NUMBER eval ( EXP e,  ENV rho);

// (def f(x 1 2) (+ x (* 2 3 )))
VALUELIST evalList ( EXPLIST el, ENV rho)
{
   //do it
   if(el == 0) return 0;

   NUMBER n = eval(el->head, rho);
   VALUELIST vl = evalList(el->tail,rho);

   return mkValuelist(n,vl);
}// evalList

NUMBER applyUserFun ( NAME nm, VALUELIST actuals)
{
    FUNDEF f;
    ENV rho;
    f = fetchFun(nm);
    rho = mkEnv(f->formals, actuals);
    return eval(f->body, rho);

}// applyUserFun

NUMBER applyCtrlOp ( int op,  EXPLIST args, ENV rho )
{
   NUMBER  n =0;
    if (op == 0){  //if statement
        if (isTrueVal(eval(args->head, rho)))
            return  eval(args->tail->head, rho);
        return  eval(args->tail->tail->head, rho);
    }

   if ( op == 1) //while statement
   {
      // do it
      while (eval(args->head, rho))
      {
         n = eval(args->tail->head, rho);
      }
      return 0; // while always returns 0, after while ends, return 0.
    };
    if( op == 2)  //set
    {
      // do it
      NAME nm = args->head->varble;
      n = eval(args->tail->head,rho);
      if(isBound(nm, rho)) assign(nm, n, rho);
      else if (isBound(nm, globalEnv))  assign(nm, n, globalEnv);
      else bindVar(nm, n,globalEnv);
    }
    if( op == 3) //begin
    {
        EXPLIST el = args;
        while(el != 0){
           n = eval(el->head, rho);
           el = el->tail;
        }
    }
    return n;
}// applyCtrlOp


// begin is used for blocks
/*

set x 10
set y 1
( while (< y x) (begin (set y (+ y 1)) (print y)) )
*/

NUMBER eval ( EXP e,  ENV rho)
{
   switch (e->etype)
   {
      case VALEXP: return (e->num);
      case VAREXP: if (isBound(e->varble, rho))
                 return fetch(e->varble, rho);
              else if ( isBound(e->varble, globalEnv))
                 return fetch(e->varble, globalEnv);
              else{
                    cout << "Undefined variable: ";
                    prName(e->varble);
                    cout <<endl;
                    exit(1);
                 }
      case APEXP:  if (e->optr > numBuiltins)
                   return  applyUserFun(e->optr, evalList(e->args, rho));
                else {
                    if ( e->optr< 4 )
                         return applyCtrlOp(e->optr, e->args, rho);
                  
                    return applyValueOp(e->optr, evalList(e->args, rho));
                    }
    }
    return 0;
} // eval


/*****************************************************************
 *                     READ-EVAL-PRINT LOOP                      *
 *****************************************************************/

int main()
{
   initNames();
   globalEnv = emptyEnv();

   quittingtime = 0;
    
   while (!quittingtime)
   {
     reader();
     if ( matches(pos, 4, (char*)"quit"))
        quittingtime = 1;
     else if( (userinput[pos] == '(') &&
            matches(skipblanks(pos+1), 6, (char* )"define")  )
     {
            prName(parseDef());
            cout <<endl;
     }
     else {
            currentExp = parseExp();
            prValue(eval(currentExp, emptyEnv() ));
            cout <<endl<<endl;
         }
    }// while
  return 0;
}

// (set x 10)



