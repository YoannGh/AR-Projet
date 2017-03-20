#define N      3    /* Number of processes */ 
#define L     10    /* Buffer size */
#define NIL (N+1)   /* for an undefined value */

#define En_SC (node[1]@in_SC)
#define Dem_SC (node[1]@ask_SC)

mtype = {req, tk};

/* le processus i recoit ses messages dans canal[i] */ 
chan canal[N] = [L] of {mtype, byte};    /* pour un message de type tk, 
                                            on mettra la valeur associee a 0 */

byte SharedVar = 0;                      /* la variable partagee */ 

inline Initialisation ( ) {
   /* initialise les variables locales, sauf reqId, val et typ_mes */
  if
    :: (id == Initial_Token_Holder) -> father = NIL; token = true;
    :: else -> father = Initial_Token_Holder; token = false;
  fi
  next = NIL;
  requesting = false;
  i = 1;

}

inline Request_CS ( ) {
  /* operations effectuees lors d'une demande de SC */
  requesting = true;
  if
    :: (father != NIL) -> canal[father]!req,id; father = NIL;
    :: else -> skip;
  fi

}

inline Release_CS ( ) {
  /* operations effectuees en sortie de SC */
  requesting = false;
  if
    :: (next != NIL) -> canal[next]!tk,id; token = false; next = NIL;
    :: else -> skip;
  fi
}

inline Receive_Request_CS ( ) {
  /* traitement de la reception d'une requete */
  if
    :: (father == NIL) ->
      if
        :: (requesting) -> next = val;
        :: else -> token = false; canal[val]!tk,id;
      fi
    :: else -> canal[father]!req,val;
  fi
  father = val;
}

inline Receive_Token () {
  /* traitement de la reception du jeton */
  token = true;       
}


proctype node(byte id; byte Initial_Token_Holder){
    
   bit requesting;    /* indique si le processus a demande la SC ou pas  */
   bit token;         /* indique si le processus possede le jeton ou non */

   byte father;       /* probable owner */
   byte next;         /* next node to whom send the token */
   byte val;          /* la valeur contenue dans le message */
   mtype typ_mes;     /* le type du message recu */
   byte reqId;        /* l'Id du demandeur, pour une requete */

   chan canal_rec = canal[id];     /* un alias pour mon canal de reception */
   xr canal_rec;                   /* je dois etre le seul a le lire */
    byte i;

   /* Chaque processus execute une boucle infinie */

   Initialisation ( );
   do
       :: ((token == true) && empty(canal_rec) && (requesting == true)) ->
	      /* on oblige le detenteur du jeton a consulter les messages recus */
        if
          :: (i%10 == 0) -> i++; break;
          :: else ->
    	      in_SC :
    	           /* acces a la ressource critique (actions sur SharedVar), 
                      puis sortie de SC */
            SharedVar++;
            printf("In SC\n");
            assert(SharedVar == 1);
            SharedVar--;
            Release_CS ( );
            i++;
        fi

       :: canal_rec ? typ_mes(val) ->
	           /* traitement du message recu */
          if
            :: (typ_mes == tk) -> Receive_Token ( );
            :: else -> Receive_Request_CS ( );
          fi

       :: (requesting == false) -> /* demander la SC */
               ask_SC :
               Request_CS ( );
   od ;
}

/* Cree un ensemble de N processus */ 

init {
   byte proc; 
   atomic {
      proc=0;
      do
         :: proc <N ->
            run node(proc, 0); 
            proc++
         :: proc == N -> break 
      od
   }
}

ltl abs_famine {
  always (Dem_SC implies (eventually En_SC))
}
