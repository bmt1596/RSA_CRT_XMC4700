/******************************************************************************/
/*                                                                            */
/*  Elektrotechnik und Informationstechnik (HAW)                              */
/*  Konzeption und Umsetzung eines Demonstrators für Fehlerangriff            */
/*  Implementierung des RSA-CRT-1024 auf Microkontrolle                       */
/*  Autor: Minh Tung Bui                                                      */
/*  Datum: 16.06.2020                                                         */
/******************************************************************************/
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flint.h"
#include <time.h>

// Globale Variable
//--------------------------------------------------------------------------------------------------------//
CLINT p_primzahl;
CLINT q_primzahl;
CLINT n_schluessel;
CLINT e_schluessel;
CLINT d_schluessel;

CLINT klar_nachricht;
CLINT rsa_verschluesseln;
CLINT rsa_entschluesseln;
CLINT signatur;
CLINT rsa_crt_entschluesseln;
//--------------------------------------------------------------------------------------------------------//

// Benötige Funktionen
//--------------------------------------------------------------------------------------------------------//
void key_nachricht_eingabe(CLINT  p_primzahl, CLINT q_primzahl, CLINT n_schluessel, CLINT e_schluessel, CLINT d_schluessel, CLINT klar_nachricht);
void q_m_algorithmus(CLINT x, CLINT k, CLINT n, CLINT result);
void rsa_crt_berechnung(CLINT p, CLINT q, CLINT n, CLINT d, CLINT m, CLINT sig);
//--------------------------------------------------------------------------------------------------------//


/******************************************************************************/
/*                                                                            */
/*  Funktion:  zur Eingabe des Schlüssel und Nachricht                        */
/*  Syntax:    void key_nachricht_eingabe(CLINT  p, CLINT q, CLINT n, CLINT e,*/
/*                      CLINT d, CLINT nachricht)     */
/*  Eingabe:   p,q,e,d (Schlüssel), nachricht (nachricht zum verschlüsseln)   */
/*  Ausgabe:   n (öffentlicher Schlüsseln),                   */
/*  Rueckgabe: null                               */
/*                                        */
/******************************************************************************/
void key_nachricht_eingabe(CLINT  p, CLINT q, CLINT n, CLINT e, CLINT d, CLINT nachricht)
{
  char* p_string;
  char* q_string;
  char* e_string;
  char* d_string;
  char* nachricht_string;

  // define the key
  // key 512 bit for p and q
  p_string = "11795317444500306937296443701637670745746525063104042626600976573633031471379047119083862849534349538550078382348191081532534475137066828256989692139486629";
  q_string = "7868644142912179714115928082505320512408119917588632421408882074064457021075596367421100729962382030653938550295854442088128490145278080967338239725652703";
  e_string = "65537";
  d_string = "46614050293034373325443877126442612949132519631281662680377524721015919390897036941887012839683698975718594758446032265203637128372009737520951105545938522460054637812967436441254594251567731220742619688242364325799203043306042718153228467130163541200019074408396350960866750372422859773584522786875458941993";
  nachricht_string = "3";
 
  /*
   *   // 32 bit
  p_string = "57287";
  q_string = "35099";

  e_string = "17";
  d_string = "354816005";
  nachricht_string = "10";
  */

 
  /*
   *  // 56 bit
  p_string = "179747107";
  q_string = "145564691";

  e_string = "17";
  d_string = "3078215501563193";
  nachricht_string = "10";
  */
  
 /*
  * // 256 bit
  p_string = "270151268997351570427300498619178698791";
  q_string = "173992259560136077954468563215860193761";
  e_string = "17";
  d_string = "16589728135019060126979788698714626009056588370954210116464117109249674429553";
  nachricht_string = "987654321";
 */
 
  // Konvertierung des Zeichenkette
  str2clint_l(p, p_string, 10);
  str2clint_l(q, q_string, 10);
  str2clint_l(e, e_string, 10);
  str2clint_l(d, d_string, 10);
  str2clint_l(nachricht, nachricht_string, 10);
  
  // öffentlicher Schlüssel n berechnen
  mul_l(p, q, n);
}

//--------------------------------------------------------------------------------------------------------//
/******************************************************************************/
/*                                                                            */
/*  Funktion:  Berechnung zum Quadrat-Multiplikation-Algorithmus              */
/*  Syntax:    void q_m_algorithmus(CLINT x, CLINT k, CLINT n, CLINT result)  */
/*  Eingabe:   x (Base), k(Exponent) , n (Divisor)                            */
/*  Ausgabe:   result (Rest)                          */
/*  Rueckgabe: null                               */
/*                                        */
/******************************************************************************/
void q_m_algorithmus(CLINT x, CLINT k, CLINT n, CLINT result)
{
  char *k_string = xclint2str_l(k, 2, 0);

  str2clint_l(result, "1", 10);
  for (int i = 0; i < (int)strlen(k_string); i++)
  {
    mmul_l(result, result, result, n);
    if (k_string[i] == '1')
    {
      mmul_l(result, x, result, n);
    }
  }

}
//--------------------------------------------------------------------------------------------------------//

/******************************************************************************/
/*                                                                            */
/*  Funktion:  Berechnung zum RSA_CRT_Algorithmus                             */
/*  Syntax:    void rsa_crt_berechnung(CLINT p, CLINT q, CLINT n, CLINT d,    */
/*           CLINT m, CLINT sig);                                             */
/*  Eingabe:   Schlüssel (p,q,n,d) , m (Nachricht)                            */
/*  Ausgabe:   sig (Signatur)                                                 */
/*  Rueckgabe: null                                                           */
/*                                        */
/******************************************************************************/
void rsa_crt_berechnung(CLINT p, CLINT q, CLINT n, CLINT d, CLINT m, CLINT sig)
{
  int vorzeichen_u;
  int vorzeichen_v;
    
  CLINT p_1;            // Ergebnis: p-1
  CLINT q_1;            // Ergebnis: q-1
  CLINT d_p;            // Ergebnis: dp = d mod(p-1)
  CLINT d_q;            // Ergebnis: dq = d mod(q-1)
  CLINT ggT;            // Größter gemeinsamer Teiler von p und q
  CLINT u;              // Faktor u
  CLINT v;              // Faktor v
  
  CLINT sig_p;          // Ergebnis sig_p = m^(dp) mod p 
  CLINT sig_q;          // Ergebnis sig_q = m^(dq) mod q 
  
  CLINT p_sigq;         // Ergebnis: p*sig_q
  CLINT u_mod_n;        // Ergebnis: u mod n
  CLINT u_p_sigq_mod_n; // Ergebnis: u*p*sigq mod n
  CLINT sub_n_u;        // Ergebnis: n-u
  
  CLINT q_sigp;         // Ergebnis: q*sig_p
  CLINT v_mod_n;        // Ergebnis: v mod n
  CLINT v_q_sigp_mod_n; // Ergebnis: v*q*sigp mod n
  CLINT sub_n_v;        // Ergebnis: n-v
  
  CLINT sum;            // Ergebnis: u*p*sigq + v*q*sigp

  cpy_l(p_1, p);        // p_1, q_1 berechnnen
  dec_l(p_1);
  cpy_l(q_1, q);
  dec_l(q_1);


  mod_l(d, p_1, d_p);   // dp berechnen
  mod_l(d, q_1, d_q);   // dq berechnen

  xgcd_l(p, q, ggT, u, &vorzeichen_u, v, &vorzeichen_v);

  q_m_algorithmus(m, d_p, p, sig_p);  //sig_p sig_q berechnen
  q_m_algorithmus(m, d_q, q, sig_q);
  
  // Betrachtung: ob u größer oder kleiner als 0 ist?
  if (vorzeichen_u >= 0)
  {
    mod_l(u, n, u_mod_n);
  }
  
  else if (vorzeichen_u < 0)
  {
    sub_l(n, u, sub_n_u);
    mod_l(sub_n_u, n, u_mod_n);
  }

  // Betrachtung: ob v größer oder kleiner als 0 ist?
  if (vorzeichen_v >= 0)
  {
    mod_l(v, n, v_mod_n);
  }
  else if (vorzeichen_v < 0)
  {
    sub_l(n, v, sub_n_v);
    mod_l(sub_n_v, n, v_mod_n);
  }

  // Berechnung u*p*sigq
  mmul_l(p, sig_q, p_sigq, n);
  mmul_l(p_sigq, u_mod_n, u_p_sigq_mod_n, n);

  // Berechnung v*q*sigp
  mmul_l(q, sig_p, q_sigp, n);
  mmul_l(q_sigp, v_mod_n, v_q_sigp_mod_n, n);

  // Berechnung u*p*sigq + v*q*sigp
  add_l(u_p_sigq_mod_n, v_q_sigp_mod_n, sum);

  // Berechnung sig = (u*p*sigq + v*q*sigp) mod n
  mod_l(sum, n, sig);
}


//--------------------------------------------------------------------------------------------------------//
// Die Setup-Funktion wird einmal ausgeführt, wenn Sie Reset drücken oder die Karte mit Strom versorgen
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  // init key
  key_nachricht_eingabe(p_primzahl, q_primzahl, n_schluessel, e_schluessel, d_schluessel, klar_nachricht);
}
//--------------------------------------------------------------------------------------------------------//


// Die Schleifenfunktion läuft immer wieder für immer
void loop() {  

      // c = m^e mod n
     q_m_algorithmus(klar_nachricht, e_schluessel, n_schluessel, rsa_verschluesseln);

      // signatur erstellen
     rsa_crt_berechnung(p_primzahl, q_primzahl, n_schluessel, d_schluessel, klar_nachricht, signatur);

    
      // LED einschalten (HIGH ist der Spannungspegel)
     digitalWrite(LED_BUILTIN, LOW);   
     delay(1000);
      // m = c^d mod n
      q_m_algorithmus(rsa_verschluesseln, d_schluessel, n_schluessel, rsa_entschluesseln);
     // Signaltur entschlüsseln
     q_m_algorithmus(signatur, e_schluessel, n_schluessel, rsa_crt_entschluesseln);
     
     digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW

     Serial.println("-----Implementierung RSA−CRT−1024-----");
     Serial.println("Primzahl p:");
     Serial.println(xclint2str_l(p_primzahl, 10, 0));
     Serial.println("Primzahl q:");
     Serial.println(xclint2str_l(q_primzahl, 10, 0));

     Serial.println("Key e:");
     Serial.println(xclint2str_l(e_schluessel, 10, 0));
     Serial.println("Key d:");
     Serial.println(xclint2str_l(d_schluessel, 10, 0));      
     Serial.println("---------------Nachricht--------------");
     Serial.println(xclint2str_l(klar_nachricht, 10, 0));
     
     Serial.println("--------------------------------------");
     Serial.println("verschluecsselte Nachricht (c):");
     Serial.println(xclint2str_l(rsa_verschluesseln, 10, 0));
     Serial.println("entschluecsselte Nachricht (m):");
     Serial.println(xclint2str_l(rsa_entschluesseln, 10, 0));
     
     Serial.println("--------------------------------------");
     Serial.println("Signatur sig:");
     Serial.println(xclint2str_l(signatur, 10, 0));
     Serial.println("entschluecsselte Nachricht (m):");
     Serial.println(xclint2str_l(rsa_crt_entschluesseln, 10, 0)); 
     Serial.println("--------------------------------------\n\n");
     
     delay(5000);
     
}
