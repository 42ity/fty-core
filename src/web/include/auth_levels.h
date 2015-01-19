/* 
 * File:   auth_levels.h
 * Author: EvgenyKlimov@eaton.com
 *
 * Created on January 16, 2015, 1:02 PM
 */

#ifndef AUTH_LEVELS_H
#define	AUTH_LEVELS_H

//extern int8_t access_auth_level;
/* // Consumers require this block in ECPP templates:
 <%request scope="global">
 int8_t access_auth_level;
 </%request>
   // Processing as a std::string may require casting into (int) first
 */
    
/* Define known auth levels */
#define AUTH_LEVEL__MIN             -2
#define AUTH_LEVEL_ERROR_INVALID    -2  // Token did not pass checks
#define AUTH_LEVEL_ERROR_EMPTY      -1  // Token was passed but empty
/* Level <= error == no valid session exists, token not accepted */
#define AUTH_LEVEL__ERROR           -1
#define AUTH_LEVEL_ANONYMOUS        0   // No token was passed
/* Level >= authorized == valid session exists */
#define AUTH_LEVEL__AUTHORIZED      1
#define AUTH_LEVEL_USER             1   // Identified as a minimal user
#define AUTH_LEVEL_POWERUSER        2   // Identified as a "power-user"
#define AUTH_LEVEL_ADMIN            3   // Identified as an administrator
#define AUTH_LEVEL__MAX             3


#endif	/* AUTH_LEVELS_H */

