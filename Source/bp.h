/*****************************************************************************/
/*                 REGISTRATION KEY SYSTEM FOR C PROGRAMMERS                 */
/*                               Version 2.20                                */
/*                                                                           */
/*                                                                           */
/*         (C) Copyright 1992, Brian Pirie. All Rights Reserved.             */
/*                                                                           */
/*                                                                           */
/*  You are granted permission to use an unmodified version of this code     */
/*  in any program, so long as your program's documentation acknowledges     */
/*  the use of this code.                                                    */
/*                                                                           */
/*  I can be contacted at -   FidoNet : 1:243/8                              */
/*                           Internet : brian@bpecomm.pinetree.org           */
/*                         Data (BBS) : +1 613 526 4466                      */
/*                             Postal : 1416 - 2201 Riverside Dr.            */
/*                                      Ottawa, Ontario                      */
/*                                      Canada                               */
/*                                      K1H 8K9                              */
/*                                                                           */
/*  To compile a program using the bp() function, simply include this file,  */
/*  and add the BP?.LIB file to your project / makefile.                     */
/*****************************************************************************/

                                          /* Prototype for the bp() function */
#ifdef __cplusplus

   extern "C" unsigned long bp(char *registration_string,
                               unsigned int security_code);

#else

   unsigned long bp(char *registration_string,
                    unsigned int security_code);

#endif

/* The bp() function returns an unsigned long integer (32 bits) registration
 * key which corresponds to the string pointed to by the registration_string
 * parameter. This string will usually be the name of the person who has
 * registered your program. Any given program which uses the bp() function
 * should pass it's own unique value in the security_code parameter. For
 * more information on using this registration key system, please see the
 * accompanying documentation, in the REGKEY.DOC file
 */
