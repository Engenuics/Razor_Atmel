#line 1 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\drivers\\utilities.c"
/***********************************************************************************************************************
File: utilities.c                                                                

Description:
Various useful functions.

API:
bool IsTimeUp(u32 *pu32SavedTick_, u32 u32Period_)
Calculates if u32Period_ time has passed (in ms) using comparison of
pu32SavedTick_ against G_u32SystemTime1ms.  
e.g. do nothing until 1 second has elapsed
u32ApplicationTimer = G_u32SystemTime1ms;
if( !IsTimeUp(&u32ApplicationTimer, 1000)
{
  // Time has elapsed so update timer for next period and do whatever
  u32ApplicationTimer = G_u32SystemTime1ms;
}


***********************************************************************************************************************/

#line 1 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\configuration.h"
/**********************************************************************************************************************
File: configuration.h      

Description:
Main configuration header file for project.  This file bridges many of the generic features of the 
firmware to the specific features of the design. The definitions should be updated
to match the target hardware.  
 
Bookmarks:
!!!!! External module peripheral assignments
@@@@@ GPIO board-specific parameters
##### Communication peripheral board-specific parameters

***********************************************************************************************************************/




/**********************************************************************************************************************
Runtime switches
***********************************************************************************************************************/

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/



/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
/* Common header files */
#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdlib.h"
/* stdlib.h standard header */
/* Copyright 2005-2010 IAR Systems AB. */





  #pragma system_include


#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"
/* ycheck.h internal checking header file. */
/* Copyright 2005-2010 IAR Systems AB. */

/* Note that there is no include guard for this header. This is intentional. */


  #pragma system_include


/* __INTRINSIC
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that intrinsic support could be turned off
 * individually for each file.
 */










/* __AEABI_PORTABILITY_INTERNAL_LEVEL
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that ABI support could be turned off/on
 * individually for each file.
 *
 * Possible values for this preprocessor symbol:
 *
 * 0 - ABI portability mode is disabled.
 *
 * 1 - ABI portability mode (version 1) is enabled.
 *
 * All other values are reserved for future use.
 */






#line 67 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"




/* A definiton for a function of what effects it has.
   NS  = no_state, i.e. it uses no internal or external state. It may write
         to errno though
   NE  = no_state, no_errno,  i.e. it uses no internal or external state,
         not even writing to errno. 
   NRx = no_read(x), i.e. it doesn't read through pointer parameter x.
   NWx = no_write(x), i.e. it doesn't write through pointer parameter x.
*/

#line 99 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"









#line 12 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdlib.h"
#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"
/* yvals.h internal configuration header file. */
/* Copyright 2001-2010 IAR Systems AB. */





  #pragma system_include


#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"
/* ycheck.h internal checking header file. */
/* Copyright 2005-2010 IAR Systems AB. */

/* Note that there is no include guard for this header. This is intentional. */


  #pragma system_include


/* __INTRINSIC
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that intrinsic support could be turned off
 * individually for each file.
 */










/* __AEABI_PORTABILITY_INTERNAL_LEVEL
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that ABI support could be turned off/on
 * individually for each file.
 *
 * Possible values for this preprocessor symbol:
 *
 * 0 - ABI portability mode is disabled.
 *
 * 1 - ABI portability mode (version 1) is enabled.
 *
 * All other values are reserved for future use.
 */






#line 67 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"

#line 12 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

                /* Convenience macros */









/* Used to refer to '__aeabi' symbols in the library. */ 


                /* Versions */










/*
 * Support for some C99 or other symbols
 *
 * This setting makes available some macros, functions, etc that are
 * beneficial.
 *
 * Default is to include them.
 *
 * Disabling this in C++ mode will not compile (some C++ functions uses C99
 * functionality).
 */


  /* Default turned on when compiling C++, EC++, or C99. */
#line 59 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"





#line 70 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

                /* Configuration */
#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"
/***************************************************
 *
 * DLib_Defaults.h is the library configuration manager.
 *
 * Copyright 2003-2010 IAR Systems AB.  
 *
 * This configuration header file performs the following tasks:
 *
 * 1. Includes the configuration header file, defined by _DLIB_CONFIG_FILE,
 *    that sets up a particular runtime environment.
 *
 * 2. Includes the product configuration header file, DLib_Product.h, that
 *    specifies default values for the product and makes sure that the
 *    configuration is valid.
 *
 * 3. Sets up default values for all remaining configuration symbols.
 *
 * This configuration header file, the one defined by _DLIB_CONFIG_FILE, and
 * DLib_Product.h configures how the runtime environment should behave. This
 * includes all system headers and the library itself, i.e. all system headers
 * includes this configuration header file, and the library has been built
 * using this configuration header file.
 *
 ***************************************************
 *
 * DO NOT MODIFY THIS FILE!
 *
 ***************************************************/





  #pragma system_include


/* Include the main configuration header file. */
#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\INC\\c\\DLib_Config_Normal.h"
/* Customer-specific DLib configuration. */
/* Copyright (C) 2003 IAR Systems.  All rights reserved. */





  #pragma system_include


/* No changes to the defaults. */

#line 40 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"
  /* _DLIB_CONFIG_FILE_STRING is the quoted variant of above */
#line 47 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"

/* Include the product specific header file. */
#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Product.h"




   #pragma system_include



/*********************************************************************
*
*       Configuration
*
*********************************************************************/

/* Wide character and multi byte character support in library.
 * This is not allowed to vary over configurations, since math-library
 * is built with wide character support.
 */


/* ARM uses the large implementation of DLib */


/* This ensures that the standard header file "string.h" includes
 * the Arm-specific file "DLib_Product_string.h". */


/* This ensures that the standard header file "fenv.h" includes
 * the Arm-specific file "DLib_Product_fenv.h". */


/* Max buffer used for swap in qsort */




/* Enable system locking  */
#line 45 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Product.h"

/* Enable AEABI support */


/* Enable rtmodel for setjump buffer size */


/* Enable parsing of hex floats */






/* Special placement for locale structures when building ropi libraries */




/* CPP-library uses software floatingpoint interface */


/* Use speedy implementation of floats (simple quad). */


/* Configure generic ELF init routines. */
#line 89 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Product.h"







#line 51 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"



/*
 * The remainder of the file sets up defaults for a number of
 * configuration symbols, each corresponds to a feature in the
 * libary.
 *
 * The value of the symbols should either be 1, if the feature should
 * be supported, or 0 if it shouldn't. (Except where otherwise
 * noted.)
 */


/*
 * Small or Large target
 *
 * This define determines whether the target is large or small. It must be 
 * setup in the DLib_Product header or in the compiler itself.
 *
 * For a small target some functionality in the library will not deliver 
 * the best available results. For instance the _accurate variants will not use
 * the extra precision packet for large arguments.
 * 
 */







/*
 * File handling
 *
 * Determines whether FILE descriptors and related functions exists or not.
 * When this feature is selected, i.e. set to 1, then FILE descriptors and
 * related functions (e.g. fprintf, fopen) exist. All files, even stdin,
 * stdout, and stderr will then be handled with a file system mechanism that
 * buffers files before accessing the lowlevel I/O interface (__open, __read,
 * __write, etc).
 *
 * If not selected, i.e. set to 0, then FILE descriptors and related functions
 * (e.g. fprintf, fopen) does not exist. All functions that normally uses
 * stderr will use stdout instead. Functions that uses stdout and stdin (like
 * printf and scanf) will access the lowlevel I/O interface directly (__open,
 * __read, __write, etc), i.e. there will not be any buffering.
 *
 * The default is not to have support for FILE descriptors.
 */





/*
 * Use static buffers for stdout
 *
 * This setting controls whether the stream stdout uses a static 80 bytes
 * buffer or uses a one byte buffer allocated in the file descriptor. This
 * setting is only applicable if the FILE descriptors are enabled above.
 *
 * Default is to use a static 80 byte buffer.
 */





/*
 * Support of locale interface
 *
 * "Locale" is the system in C that support language- and
 * contry-specific settings for a number of areas, including currency
 * symbols, date and time, and multibyte encodings.
 *
 * This setting determines whether the locale interface exist or not.
 * When this feature is selected, i.e. set to 1, the locale interface exist
 * (setlocale, etc). A number of preselected locales can be activated during
 * runtime. The preselected locales and encodings is choosen by defining any
 * number of _LOCALE_USE_xxx and _ENCODING_USE_xxx symbols. The application
 * will start with the "C" locale choosen. (Single byte encoding is always
 * supported in this mode.)
 *
 *
 * If not selected, i.e. set to 0, the locale interface (setlocale, etc) does
 * not exist. One preselected locale and one preselected encoding is then used
 * directly. That locale can not be changed during runtime. The preselected
 * locale and encoding is choosen by defining at most one of _LOCALE_USE_xxx
 * and at most one of _ENCODING_USE_xxx. The default is to use the "C" locale
 * and the single byte encoding, respectively.
 *
 * The default is not to have support for the locale interface with the "C"
 * locale and the single byte encoding.
 *
 * Supported locales
 * -----------------
 * _LOCALE_USE_C                  C standard locale (the default)
 * _LOCALE_USE_POSIX ISO-8859-1   Posix locale
 * _LOCALE_USE_CS_CZ ISO-8859-2   Czech language locale for Czech Republic
 * _LOCALE_USE_DA_DK ISO-8859-1   Danish language locale for Denmark
 * _LOCALE_USE_DA_EU ISO-8859-15  Danish language locale for Europe
 * _LOCALE_USE_DE_AT ISO-8859-1   German language locale for Austria
 * _LOCALE_USE_DE_BE ISO-8859-1   German language locale for Belgium
 * _LOCALE_USE_DE_CH ISO-8859-1   German language locale for Switzerland
 * _LOCALE_USE_DE_DE ISO-8859-1   German language locale for Germany
 * _LOCALE_USE_DE_EU ISO-8859-15  German language locale for Europe
 * _LOCALE_USE_DE_LU ISO-8859-1   German language locale for Luxemburg
 * _LOCALE_USE_EL_EU ISO-8859-7x  Greek language locale for Europe
 *                                (Euro symbol added)
 * _LOCALE_USE_EL_GR ISO-8859-7   Greek language locale for Greece
 * _LOCALE_USE_EN_AU ISO-8859-1   English language locale for Australia
 * _LOCALE_USE_EN_CA ISO-8859-1   English language locale for Canada
 * _LOCALE_USE_EN_DK ISO_8859-1   English language locale for Denmark
 * _LOCALE_USE_EN_EU ISO-8859-15  English language locale for Europe
 * _LOCALE_USE_EN_GB ISO-8859-1   English language locale for United Kingdom
 * _LOCALE_USE_EN_IE ISO-8859-1   English language locale for Ireland
 * _LOCALE_USE_EN_NZ ISO-8859-1   English language locale for New Zealand
 * _LOCALE_USE_EN_US ISO-8859-1   English language locale for USA
 * _LOCALE_USE_ES_AR ISO-8859-1   Spanish language locale for Argentina
 * _LOCALE_USE_ES_BO ISO-8859-1   Spanish language locale for Bolivia
 * _LOCALE_USE_ES_CL ISO-8859-1   Spanish language locale for Chile
 * _LOCALE_USE_ES_CO ISO-8859-1   Spanish language locale for Colombia
 * _LOCALE_USE_ES_DO ISO-8859-1   Spanish language locale for Dominican Republic
 * _LOCALE_USE_ES_EC ISO-8859-1   Spanish language locale for Equador
 * _LOCALE_USE_ES_ES ISO-8859-1   Spanish language locale for Spain
 * _LOCALE_USE_ES_EU ISO-8859-15  Spanish language locale for Europe
 * _LOCALE_USE_ES_GT ISO-8859-1   Spanish language locale for Guatemala
 * _LOCALE_USE_ES_HN ISO-8859-1   Spanish language locale for Honduras
 * _LOCALE_USE_ES_MX ISO-8859-1   Spanish language locale for Mexico
 * _LOCALE_USE_ES_PA ISO-8859-1   Spanish language locale for Panama
 * _LOCALE_USE_ES_PE ISO-8859-1   Spanish language locale for Peru
 * _LOCALE_USE_ES_PY ISO-8859-1   Spanish language locale for Paraguay
 * _LOCALE_USE_ES_SV ISO-8859-1   Spanish language locale for Salvador
 * _LOCALE_USE_ES_US ISO-8859-1   Spanish language locale for USA
 * _LOCALE_USE_ES_UY ISO-8859-1   Spanish language locale for Uruguay
 * _LOCALE_USE_ES_VE ISO-8859-1   Spanish language locale for Venezuela
 * _LOCALE_USE_ET_EE ISO-8859-1   Estonian language for Estonia
 * _LOCALE_USE_EU_ES ISO-8859-1   Basque language locale for Spain
 * _LOCALE_USE_FI_EU ISO-8859-15  Finnish language locale for Europe
 * _LOCALE_USE_FI_FI ISO-8859-1   Finnish language locale for Finland
 * _LOCALE_USE_FO_FO ISO-8859-1   Faroese language locale for Faroe Islands
 * _LOCALE_USE_FR_BE ISO-8859-1   French language locale for Belgium
 * _LOCALE_USE_FR_CA ISO-8859-1   French language locale for Canada
 * _LOCALE_USE_FR_CH ISO-8859-1   French language locale for Switzerland
 * _LOCALE_USE_FR_EU ISO-8859-15  French language locale for Europe
 * _LOCALE_USE_FR_FR ISO-8859-1   French language locale for France
 * _LOCALE_USE_FR_LU ISO-8859-1   French language locale for Luxemburg
 * _LOCALE_USE_GA_EU ISO-8859-15  Irish language locale for Europe
 * _LOCALE_USE_GA_IE ISO-8859-1   Irish language locale for Ireland
 * _LOCALE_USE_GL_ES ISO-8859-1   Galician language locale for Spain
 * _LOCALE_USE_HR_HR ISO-8859-2   Croatian language locale for Croatia
 * _LOCALE_USE_HU_HU ISO-8859-2   Hungarian language locale for Hungary
 * _LOCALE_USE_ID_ID ISO-8859-1   Indonesian language locale for Indonesia
 * _LOCALE_USE_IS_EU ISO-8859-15  Icelandic language locale for Europe
 * _LOCALE_USE_IS_IS ISO-8859-1   Icelandic language locale for Iceland
 * _LOCALE_USE_IT_EU ISO-8859-15  Italian language locale for Europe
 * _LOCALE_USE_IT_IT ISO-8859-1   Italian language locale for Italy
 * _LOCALE_USE_IW_IL ISO-8859-8   Hebrew language locale for Israel
 * _LOCALE_USE_KL_GL ISO-8859-1   Greenlandic language locale for Greenland
 * _LOCALE_USE_LT_LT   BALTIC     Lithuanian languagelocale for Lithuania
 * _LOCALE_USE_LV_LV   BALTIC     Latvian languagelocale for Latvia
 * _LOCALE_USE_NL_BE ISO-8859-1   Dutch language locale for Belgium
 * _LOCALE_USE_NL_EU ISO-8859-15  Dutch language locale for Europe
 * _LOCALE_USE_NL_NL ISO-8859-9   Dutch language locale for Netherlands
 * _LOCALE_USE_NO_EU ISO-8859-15  Norwegian language locale for Europe
 * _LOCALE_USE_NO_NO ISO-8859-1   Norwegian language locale for Norway
 * _LOCALE_USE_PL_PL ISO-8859-2   Polish language locale for Poland
 * _LOCALE_USE_PT_BR ISO-8859-1   Portugese language locale for Brazil
 * _LOCALE_USE_PT_EU ISO-8859-15  Portugese language locale for Europe
 * _LOCALE_USE_PT_PT ISO-8859-1   Portugese language locale for Portugal
 * _LOCALE_USE_RO_RO ISO-8859-2   Romanian language locale for Romania
 * _LOCALE_USE_RU_RU ISO-8859-5   Russian language locale for Russia
 * _LOCALE_USE_SL_SI ISO-8859-2   Slovenian language locale for Slovenia
 * _LOCALE_USE_SV_EU ISO-8859-15  Swedish language locale for Europe
 * _LOCALE_USE_SV_FI ISO-8859-1   Swedish language locale for Finland
 * _LOCALE_USE_SV_SE ISO-8859-1   Swedish language locale for Sweden
 * _LOCALE_USE_TR_TR ISO-8859-9   Turkish language locale for Turkey
 *
 *  Supported encodings
 *  -------------------
 * n/a                            Single byte (used if no other is defined).
 * _ENCODING_USE_UTF8             UTF8 encoding.
 */






/* We need to have the "C" locale if we have full locale support. */






/*
 * Support of multibytes in printf- and scanf-like functions
 *
 * This is the default value for _DLIB_PRINTF_MULTIBYTE and
 * _DLIB_SCANF_MULTIBYTE. See them for a description.
 *
 * Default is to not have support for multibytes in printf- and scanf-like
 * functions.
 */






/*
 * Throw handling in the EC++ library
 *
 * This setting determines what happens when the EC++ part of the library
 * fails (where a normal C++ library 'throws').
 *
 * The following alternatives exists (setting of the symbol):
 * 0                - The application does nothing, i.e. continues with the
 *                    next statement.
 * 1                - The application terminates by calling the 'abort'
 *                    function directly.
 * <anything else>  - An object of class "exception" is created.  This
 *                    object contains a string describing the problem.
 *                    This string is later emitted on "stderr" before
 *                    the application terminates by calling the 'abort'
 *                    function directly.
 *
 * Default is to do nothing.
 */






/*
 * Hexadecimal floating-point numbers in strtod
 *
 * If selected, i.e. set to 1, strtod supports C99 hexadecimal floating-point
 * numbers. This also enables hexadecimal floating-points in internal functions
 * used for converting strings and wide strings to float, double, and long
 * double.
 *
 * If not selected, i.e. set to 0, C99 hexadecimal floating-point numbers
 * aren't supported.
 *
 * Default is not to support hexadecimal floating-point numbers.
 */






/*
 * Printf configuration symbols.
 *
 * All the configuration symbols described further on controls the behaviour
 * of printf, sprintf, and the other printf variants.
 *
 * The library proves four formatters for printf: 'tiny', 'small',
 * 'large', and 'default'.  The setup in this file controls all except
 * 'tiny'.  Note that both small' and 'large' explicitly removes
 * some features.
 */

/*
 * Handle multibytes in printf
 *
 * This setting controls whether multibytes and wchar_ts are supported in
 * printf. Set to 1 to support them, otherwise set to 0.
 *
 * See _DLIB_FORMATTED_MULTIBYTE for the default setting.
 */





/*
 * Long long formatting in printf
 *
 * This setting controls long long support (%lld) in printf. Set to 1 to
 * support it, otherwise set to 0.

 * Note, if long long should not be supported and 'intmax_t' is larger than
 * an ordinary 'long', then %jd and %jn will not be supported.
 *
 * Default is to support long long formatting.
 */

#line 351 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"






/*
 * Floating-point formatting in printf
 *
 * This setting controls whether printf supports floating-point formatting.
 * Set to 1 to support them, otherwise set to 0.
 *
 * Default is to support floating-point formatting.
 */





/*
 * Hexadecimal floating-point formatting in printf
 *
 * This setting controls whether the %a format, i.e. the output of
 * floating-point numbers in the C99 hexadecimal format. Set to 1 to support
 * it, otherwise set to 0.
 *
 * Default is to support %a in printf.
 */





/*
 * Output count formatting in printf
 *
 * This setting controls whether the output count specifier (%n) is supported
 * or not in printf. Set to 1 to support it, otherwise set to 0.
 *
 * Default is to support %n in printf.
 */





/*
 * Support of qualifiers in printf
 *
 * This setting controls whether qualifiers that enlarges the input value
 * [hlLjtz] is supported in printf or not. Set to 1 to support them, otherwise
 * set to 0. See also _DLIB_PRINTF_INT_TYPE_IS_INT and
 * _DLIB_PRINTF_INT_TYPE_IS_LONG.
 *
 * Default is to support [hlLjtz] qualifiers in printf.
 */





/*
 * Support of flags in printf
 *
 * This setting controls whether flags (-+ #0) is supported in printf or not.
 * Set to 1 to support them, otherwise set to 0.
 *
 * Default is to support flags in printf.
 */





/*
 * Support widths and precisions in printf
 *
 * This setting controls whether widths and precisions are supported in printf.
 * Set to 1 to support them, otherwise set to 0.
 *
 * Default is to support widths and precisions in printf.
 */





/*
 * Support of unsigned integer formatting in printf
 *
 * This setting controls whether unsigned integer formatting is supported in
 * printf. Set to 1 to support it, otherwise set to 0.
 *
 * Default is to support unsigned integer formatting in printf.
 */





/*
 * Support of signed integer formatting in printf
 *
 * This setting controls whether signed integer formatting is supported in
 * printf. Set to 1 to support it, otherwise set to 0.
 *
 * Default is to support signed integer formatting in printf.
 */





/*
 * Support of formatting anything larger than int in printf
 *
 * This setting controls if 'int' should be used internally in printf, rather
 * than the largest existing integer type. If 'int' is used, any integer or
 * pointer type formatting use 'int' as internal type even though the
 * formatted type is larger. Set to 1 to use 'int' as internal type, otherwise
 * set to 0.
 *
 * See also next configuration.
 *
 * Default is to internally use largest existing internally type.
 */





/*
 * Support of formatting anything larger than long in printf
 *
 * This setting controls if 'long' should be used internally in printf, rather
 * than the largest existing integer type. If 'long' is used, any integer or
 * pointer type formatting use 'long' as internal type even though the
 * formatted type is larger. Set to 1 to use 'long' as internal type,
 * otherwise set to 0.
 *
 * See also previous configuration.
 *
 * Default is to internally use largest existing internally type.
 */









/*
 * Emit a char a time in printf
 *
 * This setting controls internal output handling. If selected, i.e. set to 1,
 * then printf emits one character at a time, which requires less code but
 * can be slightly slower for some types of output.
 *
 * If not selected, i.e. set to 0, then printf buffers some outputs.
 *
 * Note that it is recommended to either use full file support (see
 * _DLIB_FILE_DESCRIPTOR) or -- for debug output -- use the linker
 * option "-e__write_buffered=__write" to enable buffered I/O rather
 * than deselecting this feature.
 */






/*
 * Scanf configuration symbols.
 *
 * All the configuration symbols described here controls the
 * behaviour of scanf, sscanf, and the other scanf variants.
 *
 * The library proves three formatters for scanf: 'small', 'large',
 * and 'default'.  The setup in this file controls all, however both
 * 'small' and 'large' explicitly removes some features.
 */

/*
 * Handle multibytes in scanf
 *
 * This setting controls whether multibytes and wchar_t:s are supported in
 * scanf. Set to 1 to support them, otherwise set to 0.
 *
 * See _DLIB_FORMATTED_MULTIBYTE for the default.
 */





/*
 * Long long formatting in scanf
 *
 * This setting controls whether scanf supports long long support (%lld). It
 * also controls, if 'intmax_t' is larger than an ordinary 'long', i.e. how
 * the %jd and %jn specifiers behaves. Set to 1 to support them, otherwise set
 * to 0.
 *
 * Default is to support long long formatting in scanf.
 */

#line 566 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"





/*
 * Support widths in scanf
 *
 * This controls whether scanf supports widths. Set to 1 to support them,
 * otherwise set to 0.
 *
 * Default is to support widths in scanf.
 */





/*
 * Support qualifiers [hjltzL] in scanf
 *
 * This setting controls whether scanf supports qualifiers [hjltzL] or not. Set
 * to 1 to support them, otherwise set to 0.
 *
 * Default is to support qualifiers in scanf.
 */





/*
 * Support floating-point formatting in scanf
 *
 * This setting controls whether scanf supports floating-point formatting. Set
 * to 1 to support them, otherwise set to 0.
 *
 * Default is to support floating-point formatting in scanf.
 */





/*
 * Support output count formatting (%n)
 *
 * This setting controls whether scanf supports output count formatting (%n).
 * Set to 1 to support it, otherwise set to 0.
 *
 * Default is to support output count formatting in scanf.
 */





/*
 * Support scansets ([]) in scanf
 *
 * This setting controls whether scanf supports scansets ([]) or not. Set to 1
 * to support them, otherwise set to 0.
 *
 * Default is to support scansets in scanf.
 */





/*
 * Support signed integer formatting in scanf
 *
 * This setting controls whether scanf supports signed integer formatting or
 * not. Set to 1 to support them, otherwise set to 0.
 *
 * Default is to support signed integer formatting in scanf.
 */





/*
 * Support unsigned integer formatting in scanf
 *
 * This setting controls whether scanf supports unsigned integer formatting or
 * not. Set to 1 to support them, otherwise set to 0.
 *
 * Default is to support unsigned integer formatting in scanf.
 */





/*
 * Support assignment suppressing [*] in scanf
 *
 * This setting controls whether scanf supports assignment suppressing [*] or
 * not. Set to 1 to support them, otherwise set to 0.
 *
 * Default is to support assignment suppressing in scanf.
 */





/*
 * Handle multibytes in asctime and strftime.
 *
 * This setting controls whether multibytes and wchar_ts are
 * supported.Set to 1 to support them, otherwise set to 0.
 *
 * See _DLIB_FORMATTED_MULTIBYTE for the default setting.
 */





/*
 * True if "qsort" should be implemented using bubble sort.
 *
 * Bubble sort is less efficient than quick sort but requires less RAM
 * and ROM resources.
 */





/*
 * Set Buffert size used in qsort
 */





/*
 * The default "rand" function uses an array of 32 long:s of memory to
 * store the current state.
 *
 * The simple "rand" function uses only a single long. However, the
 * quality of the generated psuedo-random numbers are not as good as
 * the default implementation.
 */





/*
 * Wide character and multi byte character support in library.
 */





/*
 * Set attributes on the function used by the C-SPY debug interface to set a
 * breakpoint in.
 */





/*
 * Support threading in the library
 *
 * 0    No thread support
 * 1    Thread support with a, b, and d.
 * 2    Thread support with a, b, and e.
 * 3    Thread support with all thread-local storage in a dynamically allocated
 *        memory area and a, and b.
 *      a. Lock on heap accesses
 *      b. Optional lock on file accesses (see _DLIB_FILE_OP_LOCKS below)
 *      d. Use an external thread-local storage interface for all the
 *         libraries static and global variables.
 *      e. Static and global variables aren't safe for access from several
 *         threads.
 *
 * Note that if locks are used the following symbols must be defined:
 *
 *   _DLIB_THREAD_LOCK_ONCE_TYPE
 *   _DLIB_THREAD_LOCK_ONCE_MACRO(control_variable, init_function)
 *   _DLIB_THREAD_LOCK_ONCE_TYPE_INIT
 *
 * They will be used to initialize the needed locks only once. TYPE is the
 * type for the static control variable, MACRO is the expression that will be
 * evaluated at each usage of a lock, and INIT is the initializer for the
 * control variable.
 *
 * Note that if thread model 3 is used the symbol _DLIB_TLS_POINTER must be
 * defined. It is a thread local pointer to a dynamic memory area that
 * contains all used TLS variables for the library. Optionally the following
 * symbols can be defined as well (default is to use the default const data
 * and data memories):
 *
 *   _DLIB_TLS_INITIALIZER_MEMORY The memory to place the initializers for the
 *                                TLS memory area
 *   _DLIB_TLS_MEMORY             The memory to use for the TLS memory area. A
 *                                pointer to this memory must be castable to a
 *                                default pointer and back.
 *   _DLIB_TLS_REQUIRE_INIT       Set to 1 to require __cstart_init_tls
 *                                when needed to initialize the TLS data
 *                                segment for the main thread.
 *   _DLIB_TLS_SEGMENT_DATA       The name of the TLS RAM data segment
 *   _DLIB_TLS_SEGMENT_INIT       The name of the used to initialize the
 *                                TLS data segment.
 *
 * See DLib_Threads.h for a description of what interfaces needs to be
 * defined for thread support.
 */





/*
 * Used by products where one runtime library can be used by applications
 * with different data models, in order to reduce the total number of
 * libraries required. Typically, this is used when the pointer types does
 * not change over the data models used, but the placement of data variables
 * or/and constant variables do.
 *
 * If defined, this symbol is typically defined to the memory attribute that
 * is used by the runtime library. The actual define must use a
 * _Pragma("type_attribute = xxx") construct. In the header files, it is used
 * on all statically linked data objects seen by the application.
 */




#line 812 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"


/*
 * Turn on support for the Target-specific ABI. The ABI is based on the
 * ARM AEABI. A target, except ARM, may deviate from it.
 */

#line 826 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"


  /* Possible AEABI deviations */
#line 836 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"

#line 844 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"
  /*
   * The "difunc" table contains information about C++ objects that
   * should be dynamically initialized, where each entry in the table
   * represents an initialization function that should be called. When
   * the symbol _DLIB_AEABI_DIFUNC_CONTAINS_OFFSETS is true, each
   * entry in the table is encoded as an offset from the entry
   * location. When false, the entries contain the actual addresses to
   * call.
   */






/*
 * Turn on usage of a pragma to tell the linker the number of elements used
 * in a setjmp jmp_buf.
 */





/*
 * If true, the product supplies a "DLib_Product_string.h" file that
 * is included from "string.h".
 */





/*
 * Determine whether the math fma routines are fast or not.
 */




/*
 * Rtti support.
 */

#line 899 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"

/*
 * Use the "pointers to short" or "pointers to long" implementation of 
 * the basic floating point routines (like Dnorm, Dtest, Dscale, and Dunscale).
 */




/*
 * Use 64-bit long long as intermediary type in Dtest, and fabs.
 * Default is to do this if long long is 64-bits.
 */




/*
 * Favor speed versus some size enlargements in floating point functions.
 */




/*
 * Include dlmalloc as an alternative heap manager in product.
 *
 * Typically, an application will use a "malloc" heap manager that is
 * relatively small but not that efficient. An application can
 * optionally use the "dlmalloc" package, which provides a more
 * effective "malloc" heap manager, if it is included in the product
 * and supported by the settings.
 *
 * See the product documentation on how to use it, and whether or not
 * it is included in the product.
 */

  /* size_t/ptrdiff_t must be a 4 bytes unsigned integer. */
#line 943 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"





/*
 * Allow the 64-bit time_t interface?
 *
 * Default is yes if long long is 64 bits.
 */

  #pragma language = save 
  #pragma language = extended





  #pragma language = restore






/*
 * Is time_t 64 or 32 bits?
 *
 * Default is 32 bits.
 */




/*
 * Do we include math functions that demands lots of constant bytes?
 * (like erf, erfc, expm1, fma, lgamma, tgamma, and *_accurate)
 *
 */




/*
 * Set this to __weak, if supported.
 *
 */
#line 997 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Defaults.h"


/*
 * Deleted options
 *
 */







#line 73 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"











                /* Floating-point */

/*
 * Whenever a floating-point type is equal to another, we try to fold those
 * two types into one. This means that if float == double then we fold float to
 * use double internally. Example sinf(float) will use _Sin(double, uint).
 *
 * _X_FNAME is a redirector for internal support routines. The X can be
 *          D (double), F (float), or L (long double). It redirects by using
 *          another prefix. Example calls to Dtest will be __iar_Dtest,
 *          __iar_FDtest, or __iarLDtest.
 * _X_FUN   is a redirector for functions visible to the customer. As above, the
 *          X can be D, F, or L. It redirects by using another suffix. Example
 *          calls to sin will be sin, sinf, or sinl.
 * _X_TYPE  The type that one type is folded to.
 * _X_PTRCAST is a redirector for a cast operation involving a pointer.
 * _X_CAST  is a redirector for a cast involving the float type.
 *
 * _FLOAT_IS_DOUBLE signals that all internal float routines aren't needed.
 * _LONG_DOUBLE_IS_DOUBLE signals that all internal long double routines
 *                        aren't needed.
 */
#line 147 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"





                /* NAMING PROPERTIES */


/* Has support for fixed point types */




/* Has support for secure functions (printf_s, scanf_s, etc) */
/* Will not compile if enabled */
#line 170 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

/* Has support for complex C types */




/* If is Embedded C++ language */






/* If is true C++ language */






/* True C++ language setup */
#line 233 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"











                /* NAMESPACE CONTROL */
#line 292 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"









#line 308 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"








#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\xencoding_limits.h"
/* xencoding_limits.h internal header file */
/* Copyright 2003-2010 IAR Systems AB.  */





  #pragma system_include


#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"
/* ycheck.h internal checking header file. */
/* Copyright 2005-2010 IAR Systems AB. */

/* Note that there is no include guard for this header. This is intentional. */


  #pragma system_include


/* __INTRINSIC
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that intrinsic support could be turned off
 * individually for each file.
 */










/* __AEABI_PORTABILITY_INTERNAL_LEVEL
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that ABI support could be turned off/on
 * individually for each file.
 *
 * Possible values for this preprocessor symbol:
 *
 * 0 - ABI portability mode is disabled.
 *
 * 1 - ABI portability mode (version 1) is enabled.
 *
 * All other values are reserved for future use.
 */






#line 67 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"

#line 12 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\xencoding_limits.h"
#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"
/* yvals.h internal configuration header file. */
/* Copyright 2001-2010 IAR Systems AB. */

#line 707 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

/*
 * Copyright (c) 1992-2009 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.04:0576 */
#line 13 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\xencoding_limits.h"

                                /* Multibyte encoding length. */


#line 24 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\xencoding_limits.h"




#line 42 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\xencoding_limits.h"

                                /* Utility macro */














#line 317 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"



                /* FLOATING-POINT PROPERTIES */

                /* float properties */
#line 335 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

                /* double properties */
#line 360 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

                /* long double properties */
                /* (must be same as double) */




#line 382 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"


                /* INTEGER PROPERTIES */

                                /* MB_LEN_MAX */







  #pragma language=save
  #pragma language=extended
  typedef long long _Longlong;
  typedef unsigned long long _ULonglong;
  #pragma language=restore
#line 405 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"






  typedef unsigned short int _Wchart;
  typedef unsigned short int _Wintt;


#line 424 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

#line 432 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

                /* POINTER PROPERTIES */


typedef signed int  _Ptrdifft;
typedef unsigned int     _Sizet;

/* IAR doesn't support restrict  */


                /* stdarg PROPERTIES */
#line 454 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"
  typedef _VA_LIST __Va_list;



__intrinsic __nounwind void __iar_Atexit(void (*)(void));



  typedef struct
  {       /* state of a multibyte translation */
    unsigned int _Wchar;
    unsigned int _State;
  } _Mbstatet;
#line 477 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"










typedef struct
{       /* file position */

  _Longlong _Off;    /* can be system dependent */



  _Mbstatet _Wstate;
} _Fpost;







                /* THREAD AND LOCALE CONTROL */

#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Threads.h"
/***************************************************
 *
 * DLib_Threads.h is the library threads manager.
 *
 * Copyright 2004-2010 IAR Systems AB.  
 *
 * This configuration header file sets up how the thread support in the library
 * should work.
 *
 ***************************************************
 *
 * DO NOT MODIFY THIS FILE!
 *
 ***************************************************/





  #pragma system_include


/*
 * DLib can support a multithreaded environment. The preprocessor symbol 
 * _DLIB_THREAD_SUPPORT governs the support. It can be 0 (no support), 
 * 1 (currently not supported), 2 (locks only), and 3 (simulated TLS and locks).
 */

/*
 * This header sets the following symbols that governs the rest of the
 * library:
 * ------------------------------------------
 * _DLIB_MULTI_THREAD     0 No thread support
 *                        1 Multithread support
 * _DLIB_GLOBAL_VARIABLES 0 Use external TLS interface for the libraries global
 *                          and static variables
 *                        1 Use a lock for accesses to the locale and no 
 *                          security for accesses to other global and static
 *                          variables in the library
 * _DLIB_FILE_OP_LOCKS    0 No file-atomic locks
 *                        1 File-atomic locks

 * _DLIB_COMPILER_TLS     0 No Thread-Local-Storage support in the compiler
 *                        1 Thread-Local-Storage support in the compiler
 * _DLIB_TLS_QUAL         The TLS qualifier, define only if _COMPILER_TLS == 1
 *
 * _DLIB_THREAD_MACRO_SETUP_DONE Whether to use the standard definitions of
 *                               TLS macros defined in xtls.h or the definitions
 *                               are provided here.
 *                        0 Use default macros
 *                        1 Macros defined for xtls.h
 *
 * _DLIB_THREAD_LOCK_ONCE_TYPE
 *                        type for control variable in once-initialization of 
 *                        locks
 * _DLIB_THREAD_LOCK_ONCE_MACRO(control_variable, init_function)
 *                        expression that will be evaluated at each lock access
 *                        to determine if an initialization must be done
 * _DLIB_THREAD_LOCK_ONCE_TYPE_INIT
 *                        initial value for the control variable
 *
 ****************************************************************************
 * Description
 * -----------
 *
 * If locks are to be used (_DLIB_MULTI_THREAD != 0), the following options
 * has to be used in ilink: 
 *   --redirect __iar_Locksyslock=__iar_Locksyslock_mtx
 *   --redirect __iar_Unlocksyslock=__iar_Unlocksyslock_mtx
 *   --redirect __iar_Lockfilelock=__iar_Lockfilelock_mtx
 *   --redirect __iar_Unlockfilelock=__iar_Unlockfilelock_mtx
 *   --keep     __iar_Locksyslock_mtx
 * and, if C++ is used, also:
 *   --redirect __iar_Initdynamicfilelock=__iar_Initdynamicfilelock_mtx
 *   --redirect __iar_Dstdynamicfilelock=__iar_Dstdynamicfilelock_mtx
 *   --redirect __iar_Lockdynamicfilelock=__iar_Lockdynamicfilelock_mtx
 *   --redirect __iar_Unlockdynamicfilelock=__iar_Unlockdynamicfilelock_mtx
 * Xlink uses similar options (-e and -g). The following lock interface must
 * also be implemented: 
 *   typedef void *__iar_Rmtx;                   // Lock info object
 *
 *   void __iar_system_Mtxinit(__iar_Rmtx *);    // Initialize a system lock
 *   void __iar_system_Mtxdst(__iar_Rmtx *);     // Destroy a system lock
 *   void __iar_system_Mtxlock(__iar_Rmtx *);    // Lock a system lock
 *   void __iar_system_Mtxunlock(__iar_Rmtx *);  // Unlock a system lock
 * The interface handles locks for the heap, the locale, the file system
 * structure, the initialization of statics in functions, etc. 
 *
 * The following lock interface is optional to be implemented:
 *   void __iar_file_Mtxinit(__iar_Rmtx *);    // Initialize a file lock
 *   void __iar_file_Mtxdst(__iar_Rmtx *);     // Destroy a file lock
 *   void __iar_file_Mtxlock(__iar_Rmtx *);    // Lock a file lock
 *   void __iar_file_Mtxunlock(__iar_Rmtx *);  // Unlock a file lock
 * The interface handles locks for each file stream.
 * 
 * These three once-initialization symbols must also be defined, if the 
 * default initialization later on in this file doesn't work (done in 
 * DLib_product.h):
 *
 *   _DLIB_THREAD_LOCK_ONCE_TYPE
 *   _DLIB_THREAD_LOCK_ONCE_MACRO(control_variable, init_function)
 *   _DLIB_THREAD_LOCK_ONCE_TYPE_INIT
 *
 * If an external TLS interface is used, the following must
 * be defined:
 *   typedef int __iar_Tlskey_t;
 *   typedef void (*__iar_Tlsdtor_t)(void *);
 *   int __iar_Tlsalloc(__iar_Tlskey_t *, __iar_Tlsdtor_t); 
 *                                                    // Allocate a TLS element
 *   int __iar_Tlsfree(__iar_Tlskey_t);               // Free a TLS element
 *   int __iar_Tlsset(__iar_Tlskey_t, void *);        // Set a TLS element
 *   void *__iar_Tlsget(__iar_Tlskey_t);              // Get a TLS element
 *
 */

/* We don't have a compiler that supports tls declarations */



#line 157 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Threads.h"

  /* Thread support, library supports threaded variables in a user specified
     memory area, locks on heap and on FILE */

  /* See Documentation/ThreadsInternal.html for a description. */





  


#line 176 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Threads.h"





#line 187 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Threads.h"





  #pragma language=save 
  #pragma language=extended
  __intrinsic __nounwind void __iar_dlib_perthread_initialize(void  *);
  __intrinsic __nounwind void  *__iar_dlib_perthread_allocate(void);
  __intrinsic __nounwind void __iar_dlib_perthread_destroy(void);
  __intrinsic __nounwind void __iar_dlib_perthread_deallocate(void  *);









  #pragma segment = "__DLIB_PERTHREAD" 
  #pragma segment = "__DLIB_PERTHREAD_init" 


























#line 242 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Threads.h"

  /* The thread-local variable access function */
  void  *__iar_dlib_perthread_access(void  *);
  #pragma language=restore



























    /* Make sure that each destructor is inserted into _Deallocate_TLS */
  









  /* Internal function declarations. */






  





  
  typedef void *__iar_Rmtx;
  

  
  __intrinsic __nounwind void __iar_system_Mtxinit(__iar_Rmtx *m);
  __intrinsic __nounwind void __iar_system_Mtxdst(__iar_Rmtx *m);
  __intrinsic __nounwind void __iar_system_Mtxlock(__iar_Rmtx *m);
  __intrinsic __nounwind void __iar_system_Mtxunlock(__iar_Rmtx *m);

  __intrinsic __nounwind void __iar_file_Mtxinit(__iar_Rmtx *m);
  __intrinsic __nounwind void __iar_file_Mtxdst(__iar_Rmtx *m);
  __intrinsic __nounwind void __iar_file_Mtxlock(__iar_Rmtx *m);
  __intrinsic __nounwind void __iar_file_Mtxunlock(__iar_Rmtx *m);

  /* Function to destroy the locks. Should be called after atexit and 
     _Close_all. */
  __intrinsic __nounwind void __iar_clearlocks(void);


#line 323 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Threads.h"

  





  

#line 341 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Threads.h"

  typedef unsigned _Once_t;



  













#line 506 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

#line 516 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

                /* THREAD-LOCAL STORAGE */
#line 524 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"


                /* MULTITHREAD PROPERTIES */

  
  
  /* The lock interface for DLib to use. */ 
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Locksyslock_Locale(void);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Locksyslock_Malloc(void);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Locksyslock_Stream(void);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Locksyslock_Debug(void);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Locksyslock_StaticGuard(void);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Locksyslock(unsigned int);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Unlocksyslock_Locale(void);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Unlocksyslock_Malloc(void);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Unlocksyslock_Stream(void);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Unlocksyslock_Debug(void);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Unlocksyslock_StaticGuard(void);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Unlocksyslock(unsigned int);

  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Initdynamicfilelock(__iar_Rmtx *);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Dstdynamicfilelock(__iar_Rmtx *);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Lockdynamicfilelock(__iar_Rmtx *);
  _Pragma("object_attribute = __weak") __intrinsic __nounwind void __iar_Unlockdynamicfilelock(__iar_Rmtx *);
  
  
#line 564 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

                /* LOCK MACROS */
#line 572 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

#line 690 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"

                /* MISCELLANEOUS MACROS AND FUNCTIONS*/





#line 705 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\yvals.h"



/*
 * Copyright (c) 1992-2009 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.04:0576 */
#line 13 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdlib.h"
#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ysizet.h"
/* ysizet.h internal header file. */
/* Copyright 2003-2010 IAR Systems AB.  */





  #pragma system_include


#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"
/* ycheck.h internal checking header file. */
/* Copyright 2005-2010 IAR Systems AB. */

/* Note that there is no include guard for this header. This is intentional. */


  #pragma system_include


/* __INTRINSIC
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that intrinsic support could be turned off
 * individually for each file.
 */










/* __AEABI_PORTABILITY_INTERNAL_LEVEL
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that ABI support could be turned off/on
 * individually for each file.
 *
 * Possible values for this preprocessor symbol:
 *
 * 0 - ABI portability mode is disabled.
 *
 * 1 - ABI portability mode (version 1) is enabled.
 *
 * All other values are reserved for future use.
 */






#line 67 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"

#line 12 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ysizet.h"



                /* type definitions */




typedef _Sizet size_t;




typedef unsigned int __data_size_t;











#line 14 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdlib.h"


/* Module consistency. */
#pragma rtmodel="__dlib_full_locale_support",   "0"





extern int __aeabi_MB_CUR_MAX(void);


#line 34 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdlib.h"





                /* MACROS */













                /* TYPE DEFINITIONS */


  typedef _Wchart wchar_t;


typedef struct
{       /* result of int divide */
  int quot;
  int rem;
} div_t;

typedef struct
{       /* result of long divide */
  long quot;
  long rem;
} ldiv_t;



    #pragma language=save
    #pragma language=extended
    typedef struct
    {     /* result of long long divide */
      _Longlong quot;
      _Longlong rem;
    } lldiv_t;
    #pragma language=restore



                /* DECLARATIONS */
 /* low-level functions */
__intrinsic __nounwind int atexit(void (*)(void));

  __intrinsic __noreturn __nounwind void _Exit(int) ;

__intrinsic __noreturn __nounwind void exit(int) ;
__intrinsic __nounwind char * getenv(const char *);
__intrinsic __nounwind int system(const char *);



             __intrinsic __noreturn __nounwind void abort(void) ;
_Pragma("function_effects = no_state, no_errno")     __intrinsic __nounwind int abs(int);
             __intrinsic __nounwind void * calloc(size_t, size_t);
_Pragma("function_effects = no_state, no_errno")     __intrinsic __nounwind div_t div(int, int);
             __intrinsic __nounwind void free(void *);
_Pragma("function_effects = no_state, no_errno")     __intrinsic __nounwind long labs(long);
_Pragma("function_effects = no_state, no_errno")     __intrinsic __nounwind ldiv_t ldiv(long, long);


    #pragma language=save
    #pragma language=extended
    _Pragma("function_effects = no_state, no_errno") __intrinsic __nounwind long long llabs(long long);
    _Pragma("function_effects = no_state, no_errno") __intrinsic __nounwind lldiv_t lldiv(long long, long long);
    #pragma language=restore


             __intrinsic __nounwind void * malloc(size_t);
_Pragma("function_effects = no_write(1)")    __intrinsic __nounwind int mblen(const char *, size_t);
_Pragma("function_effects = no_read(1), no_write(2)") __intrinsic __nounwind size_t mbstowcs(wchar_t *, 
                                          const char *, size_t);
_Pragma("function_effects = no_read(1), no_write(2)") __intrinsic __nounwind int mbtowc(wchar_t *, const char *, 
                                     size_t);
             __intrinsic __nounwind int rand(void);
             __intrinsic __nounwind void srand(unsigned int);
             __intrinsic __nounwind void * realloc(void *, size_t);
_Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind long strtol(const char *, 
                                      char **, int);
_Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind unsigned long strtoul(const char *, char **, int);
_Pragma("function_effects = no_read(1), no_write(2)") __intrinsic __nounwind size_t wcstombs(char *, 
                                          const wchar_t *, size_t);
_Pragma("function_effects = no_read(1)")    __intrinsic __nounwind int wctomb(char *, wchar_t);


    #pragma language=save
    #pragma language=extended
    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind long long strtoll(const char *, char **, int);
    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind unsigned long long strtoull(const char *, 
                                                          char **, int);
    #pragma language=restore





#line 188 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdlib.h"



    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind unsigned long __iar_Stoul(const char *, char **, 
                                                        int);
    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind float         __iar_Stof(const char *, char **, long);
    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind double        __iar_Stod(const char *, char **, long);
    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind long double   __iar_Stold(const char *, char **, 
                                                          long);
    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind long          __iar_Stolx(const char *, char **, int, 
                                                        int *);
    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind unsigned long __iar_Stoulx(const char *, char **,
                                                         int, int *);
    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind float         __iar_Stofx(const char *, char **, 
                                                        long, int *);
    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind double        __iar_Stodx(const char *, char **, 
                                                        long, int *);
    _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind long double   __iar_Stoldx(const char *, char **, 
                                                         long, int *);

      #pragma language=save
      #pragma language=extended
      _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind _Longlong   __iar_Stoll(const char *, char **, 
                                                        int);
      _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind _ULonglong   __iar_Stoull(const char *, char **, 
                                                          int);
      _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind _Longlong    __iar_Stollx(const char *, char **, 
                                                          int, int *);
      _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind _ULonglong   __iar_Stoullx(const char *, char **, 
                                                           int, int *);
      #pragma language=restore








typedef int _Cmpfun(const void *, const void *);

_Pragma("function_effects = no_write(1,2)") __intrinsic void * bsearch(const void *, 
                                                   const void *, size_t,
                                                   size_t, _Cmpfun *);
             __intrinsic void qsort(void *, size_t, size_t, 
                                               _Cmpfun *);
             __intrinsic void __qsortbbl(void *, size_t, size_t, 
                                                    _Cmpfun *);
_Pragma("function_effects = no_write(1)")    __intrinsic __nounwind double atof(const char *);
_Pragma("function_effects = no_write(1)")    __intrinsic __nounwind int atoi(const char *);
_Pragma("function_effects = no_write(1)")    __intrinsic __nounwind long atol(const char *);


    #pragma language=save
    #pragma language=extended
    _Pragma("function_effects = no_write(1)") __intrinsic __nounwind long long atoll(const char *);
    #pragma language=restore

  _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind float strtof(const char *, 
                                         char **);
  _Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind long double strtold(const char *, char **);

_Pragma("function_effects = no_write(1), no_read(2)") __intrinsic __nounwind double strtod(const char *, 
                                        char **);
                                        

                                        
               __intrinsic __nounwind size_t __iar_Mbcurmax(void);

  _Pragma("function_effects = no_state, no_errno")     __intrinsic __nounwind int __iar_DLib_library_version(void);

  


#line 276 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdlib.h"



  
  typedef void _Atexfun(void);
  
#line 304 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdlib.h"


                /* INLINES, FOR C and C++ */
    #pragma inline=no_body
    double atof(const char *_S)
    {      /* convert string to double */
      return (__iar_Stod(_S, 0, 0));
    }

    #pragma inline=no_body
    int atoi(const char *_S)
    {      /* convert string to int */
      return ((int)__iar_Stoul(_S, 0, 10));
    }

    #pragma inline=no_body
    long atol(const char *_S)
    {      /* convert string to long */
      return ((long)__iar_Stoul(_S, 0, 10));
    }



        #pragma language=save
        #pragma language=extended
        #pragma inline=no_body
        long long atoll(const char *_S)
        {      /* convert string to long long */



            return ((long long)__iar_Stoull(_S, 0, 10));

        }
        #pragma language=restore



    #pragma inline=no_body
    double strtod(const char * _S, char ** _Endptr)
    {      /* convert string to double, with checking */
      return (__iar_Stod(_S, _Endptr, 0));
    }


      #pragma inline=no_body
      float strtof(const char * _S, char ** _Endptr)
      {      /* convert string to float, with checking */
        return (__iar_Stof(_S, _Endptr, 0));
      }

      #pragma inline=no_body
      long double strtold(const char * _S, char ** _Endptr)
      {      /* convert string to long double, with checking */
        return (__iar_Stold(_S, _Endptr, 0));
      }


    #pragma inline=no_body
    long strtol(const char * _S, char ** _Endptr, 
                int _Base)
    {      /* convert string to unsigned long, with checking */
      return (__iar_Stolx(_S, _Endptr, _Base, 0));
    }

    #pragma inline=no_body
    unsigned long strtoul(const char * _S, char ** _Endptr, 
                          int _Base)
    {      /* convert string to unsigned long, with checking */
      return (__iar_Stoul(_S, _Endptr, _Base));
    }



        #pragma language=save
        #pragma language=extended
        #pragma inline=no_body
        long long strtoll(const char * _S, char ** _Endptr,
                          int _Base)
        {      /* convert string to long long, with checking */



            return (__iar_Stoll(_S, _Endptr, _Base));

        }

        #pragma inline=no_body
        unsigned long long strtoull(const char * _S, 
                                    char ** _Endptr, int _Base)
        {      /* convert string to unsigned long long, with checking */



            return (__iar_Stoull(_S, _Endptr, _Base));

        }
        #pragma language=restore





  #pragma inline=no_body
  int abs(int i)
  {      /* compute absolute value of int argument */
    return (i < 0 ? -i : i);
  }

  #pragma inline=no_body
  long labs(long i)
  {      /* compute absolute value of long argument */
    return (i < 0 ? -i : i);
  }



      #pragma language=save
      #pragma language=extended
      #pragma inline=no_body
      long long llabs(long long i)
      {      /* compute absolute value of long long argument */
        return (i < 0 ? -i : i);
      }
      #pragma language=restore



#line 465 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdlib.h"




#line 494 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdlib.h"

/*
 * Copyright (c) 1992-2009 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.04:0576 */
#line 34 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\configuration.h"
#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\string.h"
/* string.h standard header */
/* Copyright 2009-2010 IAR Systems AB. */




  #pragma system_include


#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"
/* ycheck.h internal checking header file. */
/* Copyright 2005-2010 IAR Systems AB. */

/* Note that there is no include guard for this header. This is intentional. */


  #pragma system_include


/* __INTRINSIC
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that intrinsic support could be turned off
 * individually for each file.
 */










/* __AEABI_PORTABILITY_INTERNAL_LEVEL
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that ABI support could be turned off/on
 * individually for each file.
 *
 * Possible values for this preprocessor symbol:
 *
 * 0 - ABI portability mode is disabled.
 *
 * 1 - ABI portability mode (version 1) is enabled.
 *
 * All other values are reserved for future use.
 */






#line 67 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"

#line 11 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\string.h"
#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ysizet.h"
/* ysizet.h internal header file. */
/* Copyright 2003-2010 IAR Systems AB.  */

#line 30 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ysizet.h"







#line 13 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\string.h"

#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Product_string.h"
/**************************************************
 *
 * ARM-specific configuration for string.h in DLib.
 *
 * Copyright 2006 IAR Systems. All rights reserved.
 *
 * $Id: DLib_Product_string.h 78576 2014-05-05 13:37:27Z mats $
 *
 **************************************************/





  #pragma system_include




  
  

  /*
   * The following is pre-declared by the compiler.
   *
   * __INTRINSIC void __aeabi_memset (void *, size_t, int);
   * __INTRINSIC void __aeabi_memcpy (void *, const void *, size_t);
   * __INTRINSIC void __aeabi_memmove(void *, const void *, size_t);
   */


  /*
   * Inhibit inline definitions for routines with an effective
   * ARM-specific implementation.
   *
   * Not in Cortex-M1 and Cortex-MS1
   */



#line 47 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\DLib_Product_string.h"



  /*
   * Redirect calls to standard functions to the corresponding
   * __aeabi_X function.
   */


  #pragma inline=forced_no_body
  __intrinsic __nounwind void * memcpy(void * _D, const void * _S, size_t _N)
  {
    __aeabi_memcpy(_D, _S, _N);
    return _D;
  }


  #pragma inline=forced_no_body
  __intrinsic __nounwind void * memmove(void * _D, const void * _S, size_t _N)
  {
    __aeabi_memmove(_D, _S, _N);
    return _D;
  }


  #pragma inline=forced_no_body
  __intrinsic __nounwind void * memset(void * _D, int _C, size_t _N)
  {
    __aeabi_memset(_D, _N, _C);
    return _D;
  }

  
  



#line 16 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\string.h"



                /* macros */




                /* declarations */

_Pragma("function_effects = no_state, no_errno, no_write(1,2)")   __intrinsic __nounwind int        memcmp(const void *, const void *,
                                                size_t);
_Pragma("function_effects = no_state, no_errno, no_read(1), no_write(2), returns 1") __intrinsic __nounwind void *     memcpy(void *, 
                                                const void *, size_t);
_Pragma("function_effects = no_state, no_errno, no_read(1), no_write(2), returns 1") __intrinsic __nounwind void *     memmove(void *, const void *, size_t);
_Pragma("function_effects = no_state, no_errno, no_read(1), returns 1")    __intrinsic __nounwind void *     memset(void *, int, size_t);
_Pragma("function_effects = no_state, no_errno, no_write(2), returns 1")    __intrinsic __nounwind char *     strcat(char *, 
                                                const char *);
_Pragma("function_effects = no_state, no_errno, no_write(1,2)")   __intrinsic __nounwind int        strcmp(const char *, const char *);
_Pragma("function_effects = no_write(1,2)")     __intrinsic __nounwind int        strcoll(const char *, const char *);
_Pragma("function_effects = no_state, no_errno, no_read(1), no_write(2), returns 1") __intrinsic __nounwind char *     strcpy(char *, 
                                                const char *);
_Pragma("function_effects = no_state, no_errno, no_write(1,2)")   __intrinsic __nounwind size_t     strcspn(const char *, const char *);
                 __intrinsic __nounwind char *     strerror(int);
_Pragma("function_effects = no_state, no_errno, no_write(1)")      __intrinsic __nounwind size_t     strlen(const char *);
_Pragma("function_effects = no_state, no_errno, no_write(2), returns 1")    __intrinsic __nounwind char *     strncat(char *, 
                                                 const char *, size_t);
_Pragma("function_effects = no_state, no_errno, no_write(1,2)")   __intrinsic __nounwind int        strncmp(const char *, const char *, 
                                                 size_t);
_Pragma("function_effects = no_state, no_errno, no_read(1), no_write(2), returns 1") __intrinsic __nounwind char *     strncpy(char *, 
                                                 const char *, size_t);
_Pragma("function_effects = no_state, no_errno, no_write(1,2)")   __intrinsic __nounwind size_t     strspn(const char *, const char *);
_Pragma("function_effects = no_write(2)")        __intrinsic __nounwind char *     strtok(char *, 
                                                const char *);
_Pragma("function_effects = no_write(2)")        __intrinsic __nounwind size_t     strxfrm(char *, 
                                                 const char *, size_t);


  _Pragma("function_effects = no_write(1)")      __intrinsic __nounwind char *   strdup(const char *);
  _Pragma("function_effects = no_write(1,2)")   __intrinsic __nounwind int      strcasecmp(const char *, const char *);
  _Pragma("function_effects = no_write(1,2)")   __intrinsic __nounwind int      strncasecmp(const char *, const char *, 
                                                   size_t);
  _Pragma("function_effects = no_state, no_errno, no_write(2)")    __intrinsic __nounwind char *   strtok_r(char *, const char *, char **);
  _Pragma("function_effects = no_state, no_errno, no_write(1)")    __intrinsic __nounwind size_t   strnlen(char const *, size_t);




#line 81 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\string.h"
  _Pragma("function_effects = no_state, no_errno, no_write(1)")    __intrinsic __nounwind void *memchr(const void *_S, int _C, size_t _N);
  _Pragma("function_effects = no_state, no_errno, no_write(1)")    __intrinsic __nounwind char *strchr(const char *_S, int _C);
  _Pragma("function_effects = no_state, no_errno, no_write(1,2)") __intrinsic __nounwind char *strpbrk(const char *_S, const char *_P);
  _Pragma("function_effects = no_state, no_errno, no_write(1)")    __intrinsic __nounwind char *strrchr(const char *_S, int _C);
  _Pragma("function_effects = no_state, no_errno, no_write(1,2)") __intrinsic __nounwind char *strstr(const char *_S, const char *_P);




                /* Inline definitions. */


                /* The implementations. */

_Pragma("function_effects = no_state, no_errno, no_write(1)")    __intrinsic __nounwind void *__iar_Memchr(const void *, int, size_t);
_Pragma("function_effects = no_state, no_errno, no_write(1)")    __intrinsic __nounwind char *__iar_Strchr(const char *, int);
               __intrinsic __nounwind char *__iar_Strerror(int, char *);
_Pragma("function_effects = no_state, no_errno, no_write(1,2)") __intrinsic __nounwind char *__iar_Strpbrk(const char *, const char *);
_Pragma("function_effects = no_state, no_errno, no_write(1)")    __intrinsic __nounwind char *__iar_Strrchr(const char *, int);
_Pragma("function_effects = no_state, no_errno, no_write(1,2)") __intrinsic __nounwind char *__iar_Strstr(const char *, const char *);


                /* inlines and overloads, for C and C++ */
#line 168 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\string.h"
                /* Then the overloads for C. */
    #pragma inline
    void *memchr(const void *_S, int _C, size_t _N)
    {
      return (__iar_Memchr(_S, _C, _N));
    }

    #pragma inline
    char *strchr(const char *_S, int _C)
    {
      return (__iar_Strchr(_S, _C));
    }

    #pragma inline
    char *strpbrk(const char *_S, const char *_P)
    {
      return (__iar_Strpbrk(_S, _P));
    }

    #pragma inline
    char *strrchr(const char *_S, int _C)
    {
      return (__iar_Strrchr(_S, _C));
    }

    #pragma inline
    char *strstr(const char *_S, const char *_P)
    {
      return (__iar_Strstr(_S, _P));
    }


  #pragma inline
  char *strerror(int _Err)
  {
    return (__iar_Strerror(_Err, 0));
  }

#line 451 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\string.h"






#line 479 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\string.h"

/*
 * Copyright (c) 1992-2009 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.04:0576 */
#line 35 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\configuration.h"
#line 1 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
//  ----------------------------------------------------------------------------
//          ATMEL Microcontroller Software Support  -  ROUSSET  -
//  ----------------------------------------------------------------------------
//  Copyright (c) 2009, Atmel Corporation
// 
//  All rights reserved.
// 
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
// 
//  - Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the disclaimer below.
// 
//  Atmel's name may not be used to endorse or promote products derived from
//  this software without specific prior written permission. 
//  
//  DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
//  DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
//  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  ----------------------------------------------------------------------------
// File Name           : AT91SAM3U4.h
// Object              : AT91SAM3U4 definitions
// Generated           : AT91 SW Application Group  11/17/2009 (13:04:57)

//  ----------------------------------------------------------------------------





typedef volatile unsigned int AT91_REG;// Hardware register definition





// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR System Peripherals
// *****************************************************************************

typedef struct _AT91S_SYS {
	AT91_REG	 HSMC4_CFG; 	// Configuration Register
	AT91_REG	 HSMC4_CTRL; 	// Control Register
	AT91_REG	 HSMC4_SR; 	// Status Register
	AT91_REG	 HSMC4_IER; 	// Interrupt Enable Register
	AT91_REG	 HSMC4_IDR; 	// Interrupt Disable Register
	AT91_REG	 HSMC4_IMR; 	// Interrupt Mask Register
	AT91_REG	 HSMC4_ADDR; 	// Address Cycle Zero Register
	AT91_REG	 HSMC4_BANK; 	// Bank Register
	AT91_REG	 HSMC4_ECCCR; 	// ECC reset register
	AT91_REG	 HSMC4_ECCCMD; 	// ECC Page size register
	AT91_REG	 HSMC4_ECCSR1; 	// ECC Status register 1
	AT91_REG	 HSMC4_ECCPR0; 	// ECC Parity register 0
	AT91_REG	 HSMC4_ECCPR1; 	// ECC Parity register 1
	AT91_REG	 HSMC4_ECCSR2; 	// ECC Status register 2
	AT91_REG	 HSMC4_ECCPR2; 	// ECC Parity register 2
	AT91_REG	 HSMC4_ECCPR3; 	// ECC Parity register 3
	AT91_REG	 HSMC4_ECCPR4; 	// ECC Parity register 4
	AT91_REG	 HSMC4_ECCPR5; 	// ECC Parity register 5
	AT91_REG	 HSMC4_ECCPR6; 	// ECC Parity register 6
	AT91_REG	 HSMC4_ECCPR7; 	// ECC Parity register 7
	AT91_REG	 HSMC4_ECCPR8; 	// ECC Parity register 8
	AT91_REG	 HSMC4_ECCPR9; 	// ECC Parity register 9
	AT91_REG	 HSMC4_ECCPR10; 	// ECC Parity register 10
	AT91_REG	 HSMC4_ECCPR11; 	// ECC Parity register 11
	AT91_REG	 HSMC4_ECCPR12; 	// ECC Parity register 12
	AT91_REG	 HSMC4_ECCPR13; 	// ECC Parity register 13
	AT91_REG	 HSMC4_ECCPR14; 	// ECC Parity register 14
	AT91_REG	 HSMC4_Eccpr15; 	// ECC Parity register 15
	AT91_REG	 Reserved0[40]; 	// 
	AT91_REG	 HSMC4_OCMS; 	// OCMS MODE register
	AT91_REG	 HSMC4_KEY1; 	// KEY1 Register
	AT91_REG	 HSMC4_KEY2; 	// KEY2 Register
	AT91_REG	 Reserved1[50]; 	// 
	AT91_REG	 HSMC4_WPCR; 	// Write Protection Control register
	AT91_REG	 HSMC4_WPSR; 	// Write Protection Status Register
	AT91_REG	 HSMC4_ADDRSIZE; 	// Write Protection Status Register
	AT91_REG	 HSMC4_IPNAME1; 	// Write Protection Status Register
	AT91_REG	 HSMC4_IPNAME2; 	// Write Protection Status Register
	AT91_REG	 HSMC4_FEATURES; 	// Write Protection Status Register
	AT91_REG	 HSMC4_VER; 	// HSMC4 Version Register
	AT91_REG	 HMATRIX_MCFG0; 	//  Master Configuration Register 0 : ARM I and D
	AT91_REG	 HMATRIX_MCFG1; 	//  Master Configuration Register 1 : ARM S
	AT91_REG	 HMATRIX_MCFG2; 	//  Master Configuration Register 2
	AT91_REG	 HMATRIX_MCFG3; 	//  Master Configuration Register 3
	AT91_REG	 HMATRIX_MCFG4; 	//  Master Configuration Register 4
	AT91_REG	 HMATRIX_MCFG5; 	//  Master Configuration Register 5
	AT91_REG	 HMATRIX_MCFG6; 	//  Master Configuration Register 6
	AT91_REG	 HMATRIX_MCFG7; 	//  Master Configuration Register 7
	AT91_REG	 Reserved2[8]; 	// 
	AT91_REG	 HMATRIX_SCFG0; 	//  Slave Configuration Register 0
	AT91_REG	 HMATRIX_SCFG1; 	//  Slave Configuration Register 1
	AT91_REG	 HMATRIX_SCFG2; 	//  Slave Configuration Register 2
	AT91_REG	 HMATRIX_SCFG3; 	//  Slave Configuration Register 3
	AT91_REG	 HMATRIX_SCFG4; 	//  Slave Configuration Register 4
	AT91_REG	 HMATRIX_SCFG5; 	//  Slave Configuration Register 5
	AT91_REG	 HMATRIX_SCFG6; 	//  Slave Configuration Register 6
	AT91_REG	 HMATRIX_SCFG7; 	//  Slave Configuration Register 5
	AT91_REG	 HMATRIX_SCFG8; 	//  Slave Configuration Register 8
	AT91_REG	 HMATRIX_SCFG9; 	//  Slave Configuration Register 9
	AT91_REG	 Reserved3[42]; 	// 
	AT91_REG	 HMATRIX_SFR0 ; 	//  Special Function Register 0
	AT91_REG	 HMATRIX_SFR1 ; 	//  Special Function Register 1
	AT91_REG	 HMATRIX_SFR2 ; 	//  Special Function Register 2
	AT91_REG	 HMATRIX_SFR3 ; 	//  Special Function Register 3
	AT91_REG	 HMATRIX_SFR4 ; 	//  Special Function Register 4
	AT91_REG	 HMATRIX_SFR5 ; 	//  Special Function Register 5
	AT91_REG	 HMATRIX_SFR6 ; 	//  Special Function Register 6
	AT91_REG	 HMATRIX_SFR7 ; 	//  Special Function Register 7
	AT91_REG	 HMATRIX_SFR8 ; 	//  Special Function Register 8
	AT91_REG	 HMATRIX_SFR9 ; 	//  Special Function Register 9
	AT91_REG	 HMATRIX_SFR10; 	//  Special Function Register 10
	AT91_REG	 HMATRIX_SFR11; 	//  Special Function Register 11
	AT91_REG	 HMATRIX_SFR12; 	//  Special Function Register 12
	AT91_REG	 HMATRIX_SFR13; 	//  Special Function Register 13
	AT91_REG	 HMATRIX_SFR14; 	//  Special Function Register 14
	AT91_REG	 HMATRIX_SFR15; 	//  Special Function Register 15
	AT91_REG	 Reserved4[39]; 	// 
	AT91_REG	 HMATRIX_ADDRSIZE; 	// HMATRIX2 ADDRSIZE REGISTER 
	AT91_REG	 HMATRIX_IPNAME1; 	// HMATRIX2 IPNAME1 REGISTER 
	AT91_REG	 HMATRIX_IPNAME2; 	// HMATRIX2 IPNAME2 REGISTER 
	AT91_REG	 HMATRIX_FEATURES; 	// HMATRIX2 FEATURES REGISTER 
	AT91_REG	 HMATRIX_VER; 	// HMATRIX2 VERSION REGISTER 
	AT91_REG	 PMC_SCER; 	// System Clock Enable Register
	AT91_REG	 PMC_SCDR; 	// System Clock Disable Register
	AT91_REG	 PMC_SCSR; 	// System Clock Status Register
	AT91_REG	 Reserved5[1]; 	// 
	AT91_REG	 PMC_PCER; 	// Peripheral Clock Enable Register
	AT91_REG	 PMC_PCDR; 	// Peripheral Clock Disable Register
	AT91_REG	 PMC_PCSR; 	// Peripheral Clock Status Register
	AT91_REG	 PMC_UCKR; 	// UTMI Clock Configuration Register
	AT91_REG	 PMC_MOR; 	// Main Oscillator Register
	AT91_REG	 PMC_MCFR; 	// Main Clock  Frequency Register
	AT91_REG	 PMC_PLLAR; 	// PLL Register
	AT91_REG	 Reserved6[1]; 	// 
	AT91_REG	 PMC_MCKR; 	// Master Clock Register
	AT91_REG	 Reserved7[3]; 	// 
	AT91_REG	 PMC_PCKR[8]; 	// Programmable Clock Register
	AT91_REG	 PMC_IER; 	// Interrupt Enable Register
	AT91_REG	 PMC_IDR; 	// Interrupt Disable Register
	AT91_REG	 PMC_SR; 	// Status Register
	AT91_REG	 PMC_IMR; 	// Interrupt Mask Register
	AT91_REG	 PMC_FSMR; 	// Fast Startup Mode Register
	AT91_REG	 PMC_FSPR; 	// Fast Startup Polarity Register
	AT91_REG	 PMC_FOCR; 	// Fault Output Clear Register
	AT91_REG	 Reserved8[28]; 	// 
	AT91_REG	 PMC_ADDRSIZE; 	// PMC ADDRSIZE REGISTER 
	AT91_REG	 PMC_IPNAME1; 	// PMC IPNAME1 REGISTER 
	AT91_REG	 PMC_IPNAME2; 	// PMC IPNAME2 REGISTER 
	AT91_REG	 PMC_FEATURES; 	// PMC FEATURES REGISTER 
	AT91_REG	 PMC_VER; 	// APMC VERSION REGISTER
	AT91_REG	 Reserved9[64]; 	// 
	AT91_REG	 DBGU_CR; 	// Control Register
	AT91_REG	 DBGU_MR; 	// Mode Register
	AT91_REG	 DBGU_IER; 	// Interrupt Enable Register
	AT91_REG	 DBGU_IDR; 	// Interrupt Disable Register
	AT91_REG	 DBGU_IMR; 	// Interrupt Mask Register
	AT91_REG	 DBGU_CSR; 	// Channel Status Register
	AT91_REG	 DBGU_RHR; 	// Receiver Holding Register
	AT91_REG	 DBGU_THR; 	// Transmitter Holding Register
	AT91_REG	 DBGU_BRGR; 	// Baud Rate Generator Register
	AT91_REG	 Reserved10[9]; 	// 
	AT91_REG	 DBGU_FNTR; 	// Force NTRST Register
	AT91_REG	 Reserved11[40]; 	// 
	AT91_REG	 DBGU_ADDRSIZE; 	// DBGU ADDRSIZE REGISTER 
	AT91_REG	 DBGU_IPNAME1; 	// DBGU IPNAME1 REGISTER 
	AT91_REG	 DBGU_IPNAME2; 	// DBGU IPNAME2 REGISTER 
	AT91_REG	 DBGU_FEATURES; 	// DBGU FEATURES REGISTER 
	AT91_REG	 DBGU_VER; 	// DBGU VERSION REGISTER 
	AT91_REG	 DBGU_RPR; 	// Receive Pointer Register
	AT91_REG	 DBGU_RCR; 	// Receive Counter Register
	AT91_REG	 DBGU_TPR; 	// Transmit Pointer Register
	AT91_REG	 DBGU_TCR; 	// Transmit Counter Register
	AT91_REG	 DBGU_RNPR; 	// Receive Next Pointer Register
	AT91_REG	 DBGU_RNCR; 	// Receive Next Counter Register
	AT91_REG	 DBGU_TNPR; 	// Transmit Next Pointer Register
	AT91_REG	 DBGU_TNCR; 	// Transmit Next Counter Register
	AT91_REG	 DBGU_PTCR; 	// PDC Transfer Control Register
	AT91_REG	 DBGU_PTSR; 	// PDC Transfer Status Register
	AT91_REG	 Reserved12[6]; 	// 
	AT91_REG	 DBGU_CIDR; 	// Chip ID Register
	AT91_REG	 DBGU_EXID; 	// Chip ID Extension Register
	AT91_REG	 Reserved13[46]; 	// 
	AT91_REG	 EFC0_FMR; 	// EFC Flash Mode Register
	AT91_REG	 EFC0_FCR; 	// EFC Flash Command Register
	AT91_REG	 EFC0_FSR; 	// EFC Flash Status Register
	AT91_REG	 EFC0_FRR; 	// EFC Flash Result Register
	AT91_REG	 Reserved14[1]; 	// 
	AT91_REG	 EFC0_FVR; 	// EFC Flash Version Register
	AT91_REG	 Reserved15[122]; 	// 
	AT91_REG	 EFC1_FMR; 	// EFC Flash Mode Register
	AT91_REG	 EFC1_FCR; 	// EFC Flash Command Register
	AT91_REG	 EFC1_FSR; 	// EFC Flash Status Register
	AT91_REG	 EFC1_FRR; 	// EFC Flash Result Register
	AT91_REG	 Reserved16[1]; 	// 
	AT91_REG	 EFC1_FVR; 	// EFC Flash Version Register
	AT91_REG	 Reserved17[122]; 	// 
	AT91_REG	 PIOA_PER; 	// PIO Enable Register
	AT91_REG	 PIOA_PDR; 	// PIO Disable Register
	AT91_REG	 PIOA_PSR; 	// PIO Status Register
	AT91_REG	 Reserved18[1]; 	// 
	AT91_REG	 PIOA_OER; 	// Output Enable Register
	AT91_REG	 PIOA_ODR; 	// Output Disable Registerr
	AT91_REG	 PIOA_OSR; 	// Output Status Register
	AT91_REG	 Reserved19[1]; 	// 
	AT91_REG	 PIOA_IFER; 	// Input Filter Enable Register
	AT91_REG	 PIOA_IFDR; 	// Input Filter Disable Register
	AT91_REG	 PIOA_IFSR; 	// Input Filter Status Register
	AT91_REG	 Reserved20[1]; 	// 
	AT91_REG	 PIOA_SODR; 	// Set Output Data Register
	AT91_REG	 PIOA_CODR; 	// Clear Output Data Register
	AT91_REG	 PIOA_ODSR; 	// Output Data Status Register
	AT91_REG	 PIOA_PDSR; 	// Pin Data Status Register
	AT91_REG	 PIOA_IER; 	// Interrupt Enable Register
	AT91_REG	 PIOA_IDR; 	// Interrupt Disable Register
	AT91_REG	 PIOA_IMR; 	// Interrupt Mask Register
	AT91_REG	 PIOA_ISR; 	// Interrupt Status Register
	AT91_REG	 PIOA_MDER; 	// Multi-driver Enable Register
	AT91_REG	 PIOA_MDDR; 	// Multi-driver Disable Register
	AT91_REG	 PIOA_MDSR; 	// Multi-driver Status Register
	AT91_REG	 Reserved21[1]; 	// 
	AT91_REG	 PIOA_PPUDR; 	// Pull-up Disable Register
	AT91_REG	 PIOA_PPUER; 	// Pull-up Enable Register
	AT91_REG	 PIOA_PPUSR; 	// Pull-up Status Register
	AT91_REG	 Reserved22[1]; 	// 
	AT91_REG	 PIOA_ABSR; 	// Peripheral AB Select Register
	AT91_REG	 Reserved23[3]; 	// 
	AT91_REG	 PIOA_SCIFSR; 	// System Clock Glitch Input Filter Select Register
	AT91_REG	 PIOA_DIFSR; 	// Debouncing Input Filter Select Register
	AT91_REG	 PIOA_IFDGSR; 	// Glitch or Debouncing Input Filter Clock Selection Status Register
	AT91_REG	 PIOA_SCDR; 	// Slow Clock Divider Debouncing Register
	AT91_REG	 Reserved24[4]; 	// 
	AT91_REG	 PIOA_OWER; 	// Output Write Enable Register
	AT91_REG	 PIOA_OWDR; 	// Output Write Disable Register
	AT91_REG	 PIOA_OWSR; 	// Output Write Status Register
	AT91_REG	 Reserved25[1]; 	// 
	AT91_REG	 PIOA_AIMER; 	// Additional Interrupt Modes Enable Register
	AT91_REG	 PIOA_AIMDR; 	// Additional Interrupt Modes Disables Register
	AT91_REG	 PIOA_AIMMR; 	// Additional Interrupt Modes Mask Register
	AT91_REG	 Reserved26[1]; 	// 
	AT91_REG	 PIOA_ESR; 	// Edge Select Register
	AT91_REG	 PIOA_LSR; 	// Level Select Register
	AT91_REG	 PIOA_ELSR; 	// Edge/Level Status Register
	AT91_REG	 Reserved27[1]; 	// 
	AT91_REG	 PIOA_FELLSR; 	// Falling Edge/Low Level Select Register
	AT91_REG	 PIOA_REHLSR; 	// Rising Edge/ High Level Select Register
	AT91_REG	 PIOA_FRLHSR; 	// Fall/Rise - Low/High Status Register
	AT91_REG	 Reserved28[1]; 	// 
	AT91_REG	 PIOA_LOCKSR; 	// Lock Status Register
	AT91_REG	 Reserved29[6]; 	// 
	AT91_REG	 PIOA_VER; 	// PIO VERSION REGISTER 
	AT91_REG	 Reserved30[8]; 	// 
	AT91_REG	 PIOA_KER; 	// Keypad Controller Enable Register
	AT91_REG	 PIOA_KRCR; 	// Keypad Controller Row Column Register
	AT91_REG	 PIOA_KDR; 	// Keypad Controller Debouncing Register
	AT91_REG	 Reserved31[1]; 	// 
	AT91_REG	 PIOA_KIER; 	// Keypad Controller Interrupt Enable Register
	AT91_REG	 PIOA_KIDR; 	// Keypad Controller Interrupt Disable Register
	AT91_REG	 PIOA_KIMR; 	// Keypad Controller Interrupt Mask Register
	AT91_REG	 PIOA_KSR; 	// Keypad Controller Status Register
	AT91_REG	 PIOA_KKPR; 	// Keypad Controller Key Press Register
	AT91_REG	 PIOA_KKRR; 	// Keypad Controller Key Release Register
	AT91_REG	 Reserved32[46]; 	// 
	AT91_REG	 PIOB_PER; 	// PIO Enable Register
	AT91_REG	 PIOB_PDR; 	// PIO Disable Register
	AT91_REG	 PIOB_PSR; 	// PIO Status Register
	AT91_REG	 Reserved33[1]; 	// 
	AT91_REG	 PIOB_OER; 	// Output Enable Register
	AT91_REG	 PIOB_ODR; 	// Output Disable Registerr
	AT91_REG	 PIOB_OSR; 	// Output Status Register
	AT91_REG	 Reserved34[1]; 	// 
	AT91_REG	 PIOB_IFER; 	// Input Filter Enable Register
	AT91_REG	 PIOB_IFDR; 	// Input Filter Disable Register
	AT91_REG	 PIOB_IFSR; 	// Input Filter Status Register
	AT91_REG	 Reserved35[1]; 	// 
	AT91_REG	 PIOB_SODR; 	// Set Output Data Register
	AT91_REG	 PIOB_CODR; 	// Clear Output Data Register
	AT91_REG	 PIOB_ODSR; 	// Output Data Status Register
	AT91_REG	 PIOB_PDSR; 	// Pin Data Status Register
	AT91_REG	 PIOB_IER; 	// Interrupt Enable Register
	AT91_REG	 PIOB_IDR; 	// Interrupt Disable Register
	AT91_REG	 PIOB_IMR; 	// Interrupt Mask Register
	AT91_REG	 PIOB_ISR; 	// Interrupt Status Register
	AT91_REG	 PIOB_MDER; 	// Multi-driver Enable Register
	AT91_REG	 PIOB_MDDR; 	// Multi-driver Disable Register
	AT91_REG	 PIOB_MDSR; 	// Multi-driver Status Register
	AT91_REG	 Reserved36[1]; 	// 
	AT91_REG	 PIOB_PPUDR; 	// Pull-up Disable Register
	AT91_REG	 PIOB_PPUER; 	// Pull-up Enable Register
	AT91_REG	 PIOB_PPUSR; 	// Pull-up Status Register
	AT91_REG	 Reserved37[1]; 	// 
	AT91_REG	 PIOB_ABSR; 	// Peripheral AB Select Register
	AT91_REG	 Reserved38[3]; 	// 
	AT91_REG	 PIOB_SCIFSR; 	// System Clock Glitch Input Filter Select Register
	AT91_REG	 PIOB_DIFSR; 	// Debouncing Input Filter Select Register
	AT91_REG	 PIOB_IFDGSR; 	// Glitch or Debouncing Input Filter Clock Selection Status Register
	AT91_REG	 PIOB_SCDR; 	// Slow Clock Divider Debouncing Register
	AT91_REG	 Reserved39[4]; 	// 
	AT91_REG	 PIOB_OWER; 	// Output Write Enable Register
	AT91_REG	 PIOB_OWDR; 	// Output Write Disable Register
	AT91_REG	 PIOB_OWSR; 	// Output Write Status Register
	AT91_REG	 Reserved40[1]; 	// 
	AT91_REG	 PIOB_AIMER; 	// Additional Interrupt Modes Enable Register
	AT91_REG	 PIOB_AIMDR; 	// Additional Interrupt Modes Disables Register
	AT91_REG	 PIOB_AIMMR; 	// Additional Interrupt Modes Mask Register
	AT91_REG	 Reserved41[1]; 	// 
	AT91_REG	 PIOB_ESR; 	// Edge Select Register
	AT91_REG	 PIOB_LSR; 	// Level Select Register
	AT91_REG	 PIOB_ELSR; 	// Edge/Level Status Register
	AT91_REG	 Reserved42[1]; 	// 
	AT91_REG	 PIOB_FELLSR; 	// Falling Edge/Low Level Select Register
	AT91_REG	 PIOB_REHLSR; 	// Rising Edge/ High Level Select Register
	AT91_REG	 PIOB_FRLHSR; 	// Fall/Rise - Low/High Status Register
	AT91_REG	 Reserved43[1]; 	// 
	AT91_REG	 PIOB_LOCKSR; 	// Lock Status Register
	AT91_REG	 Reserved44[6]; 	// 
	AT91_REG	 PIOB_VER; 	// PIO VERSION REGISTER 
	AT91_REG	 Reserved45[8]; 	// 
	AT91_REG	 PIOB_KER; 	// Keypad Controller Enable Register
	AT91_REG	 PIOB_KRCR; 	// Keypad Controller Row Column Register
	AT91_REG	 PIOB_KDR; 	// Keypad Controller Debouncing Register
	AT91_REG	 Reserved46[1]; 	// 
	AT91_REG	 PIOB_KIER; 	// Keypad Controller Interrupt Enable Register
	AT91_REG	 PIOB_KIDR; 	// Keypad Controller Interrupt Disable Register
	AT91_REG	 PIOB_KIMR; 	// Keypad Controller Interrupt Mask Register
	AT91_REG	 PIOB_KSR; 	// Keypad Controller Status Register
	AT91_REG	 PIOB_KKPR; 	// Keypad Controller Key Press Register
	AT91_REG	 PIOB_KKRR; 	// Keypad Controller Key Release Register
	AT91_REG	 Reserved47[46]; 	// 
	AT91_REG	 PIOC_PER; 	// PIO Enable Register
	AT91_REG	 PIOC_PDR; 	// PIO Disable Register
	AT91_REG	 PIOC_PSR; 	// PIO Status Register
	AT91_REG	 Reserved48[1]; 	// 
	AT91_REG	 PIOC_OER; 	// Output Enable Register
	AT91_REG	 PIOC_ODR; 	// Output Disable Registerr
	AT91_REG	 PIOC_OSR; 	// Output Status Register
	AT91_REG	 Reserved49[1]; 	// 
	AT91_REG	 PIOC_IFER; 	// Input Filter Enable Register
	AT91_REG	 PIOC_IFDR; 	// Input Filter Disable Register
	AT91_REG	 PIOC_IFSR; 	// Input Filter Status Register
	AT91_REG	 Reserved50[1]; 	// 
	AT91_REG	 PIOC_SODR; 	// Set Output Data Register
	AT91_REG	 PIOC_CODR; 	// Clear Output Data Register
	AT91_REG	 PIOC_ODSR; 	// Output Data Status Register
	AT91_REG	 PIOC_PDSR; 	// Pin Data Status Register
	AT91_REG	 PIOC_IER; 	// Interrupt Enable Register
	AT91_REG	 PIOC_IDR; 	// Interrupt Disable Register
	AT91_REG	 PIOC_IMR; 	// Interrupt Mask Register
	AT91_REG	 PIOC_ISR; 	// Interrupt Status Register
	AT91_REG	 PIOC_MDER; 	// Multi-driver Enable Register
	AT91_REG	 PIOC_MDDR; 	// Multi-driver Disable Register
	AT91_REG	 PIOC_MDSR; 	// Multi-driver Status Register
	AT91_REG	 Reserved51[1]; 	// 
	AT91_REG	 PIOC_PPUDR; 	// Pull-up Disable Register
	AT91_REG	 PIOC_PPUER; 	// Pull-up Enable Register
	AT91_REG	 PIOC_PPUSR; 	// Pull-up Status Register
	AT91_REG	 Reserved52[1]; 	// 
	AT91_REG	 PIOC_ABSR; 	// Peripheral AB Select Register
	AT91_REG	 Reserved53[3]; 	// 
	AT91_REG	 PIOC_SCIFSR; 	// System Clock Glitch Input Filter Select Register
	AT91_REG	 PIOC_DIFSR; 	// Debouncing Input Filter Select Register
	AT91_REG	 PIOC_IFDGSR; 	// Glitch or Debouncing Input Filter Clock Selection Status Register
	AT91_REG	 PIOC_SCDR; 	// Slow Clock Divider Debouncing Register
	AT91_REG	 Reserved54[4]; 	// 
	AT91_REG	 PIOC_OWER; 	// Output Write Enable Register
	AT91_REG	 PIOC_OWDR; 	// Output Write Disable Register
	AT91_REG	 PIOC_OWSR; 	// Output Write Status Register
	AT91_REG	 Reserved55[1]; 	// 
	AT91_REG	 PIOC_AIMER; 	// Additional Interrupt Modes Enable Register
	AT91_REG	 PIOC_AIMDR; 	// Additional Interrupt Modes Disables Register
	AT91_REG	 PIOC_AIMMR; 	// Additional Interrupt Modes Mask Register
	AT91_REG	 Reserved56[1]; 	// 
	AT91_REG	 PIOC_ESR; 	// Edge Select Register
	AT91_REG	 PIOC_LSR; 	// Level Select Register
	AT91_REG	 PIOC_ELSR; 	// Edge/Level Status Register
	AT91_REG	 Reserved57[1]; 	// 
	AT91_REG	 PIOC_FELLSR; 	// Falling Edge/Low Level Select Register
	AT91_REG	 PIOC_REHLSR; 	// Rising Edge/ High Level Select Register
	AT91_REG	 PIOC_FRLHSR; 	// Fall/Rise - Low/High Status Register
	AT91_REG	 Reserved58[1]; 	// 
	AT91_REG	 PIOC_LOCKSR; 	// Lock Status Register
	AT91_REG	 Reserved59[6]; 	// 
	AT91_REG	 PIOC_VER; 	// PIO VERSION REGISTER 
	AT91_REG	 Reserved60[8]; 	// 
	AT91_REG	 PIOC_KER; 	// Keypad Controller Enable Register
	AT91_REG	 PIOC_KRCR; 	// Keypad Controller Row Column Register
	AT91_REG	 PIOC_KDR; 	// Keypad Controller Debouncing Register
	AT91_REG	 Reserved61[1]; 	// 
	AT91_REG	 PIOC_KIER; 	// Keypad Controller Interrupt Enable Register
	AT91_REG	 PIOC_KIDR; 	// Keypad Controller Interrupt Disable Register
	AT91_REG	 PIOC_KIMR; 	// Keypad Controller Interrupt Mask Register
	AT91_REG	 PIOC_KSR; 	// Keypad Controller Status Register
	AT91_REG	 PIOC_KKPR; 	// Keypad Controller Key Press Register
	AT91_REG	 PIOC_KKRR; 	// Keypad Controller Key Release Register
	AT91_REG	 Reserved62[46]; 	// 
	AT91_REG	 RSTC_RCR; 	// Reset Control Register
	AT91_REG	 RSTC_RSR; 	// Reset Status Register
	AT91_REG	 RSTC_RMR; 	// Reset Mode Register
	AT91_REG	 Reserved63[1]; 	// 
    AT91_REG SUPC_CR;   // Supply Controller Control Register
    AT91_REG SUPC_SMMR; // Supply Controller Supply Monitor Mode Register
    AT91_REG SUPC_MR;   // Supply Controller Mode Register
    AT91_REG SUPC_WUMR; // Supply Controller Wake Up Mode Register
    AT91_REG SUPC_WUIR; // Supply Controller Wake Up Inputs Register
    AT91_REG SUPC_SR;   // Supply Controller Status Register
	AT91_REG	 Reserved64[2]; 	// 

	AT91_REG	 RTTC_RTMR; 	// Real-time Mode Register
	AT91_REG	 RTTC_RTAR; 	// Real-time Alarm Register
	AT91_REG	 RTTC_RTVR; 	// Real-time Value Register
	AT91_REG	 RTTC_RTSR; 	// Real-time Status Register
	AT91_REG	 Reserved65[4]; 	// 
	AT91_REG	 WDTC_WDCR; 	// Watchdog Control Register
	AT91_REG	 WDTC_WDMR; 	// Watchdog Mode Register
	AT91_REG	 WDTC_WDSR; 	// Watchdog Status Register
	AT91_REG	 Reserved66[1]; 	// 
	AT91_REG	 RTC_CR; 	// Control Register
	AT91_REG	 RTC_MR; 	// Mode Register
	AT91_REG	 RTC_TIMR; 	// Time Register
	AT91_REG	 RTC_CALR; 	// Calendar Register
	AT91_REG	 RTC_TIMALR; 	// Time Alarm Register
	AT91_REG	 RTC_CALALR; 	// Calendar Alarm Register
	AT91_REG	 RTC_SR; 	// Status Register
	AT91_REG	 RTC_SCCR; 	// Status Clear Command Register
	AT91_REG	 RTC_IER; 	// Interrupt Enable Register
	AT91_REG	 RTC_IDR; 	// Interrupt Disable Register
	AT91_REG	 RTC_IMR; 	// Interrupt Mask Register
	AT91_REG	 RTC_VER; 	// Valid Entry Register
	AT91_REG	 SYS_GPBR[8]; 	// General Purpose Register
	AT91_REG	 Reserved67[19]; 	// 
	AT91_REG	 RSTC_VER; 	// Version Register
} AT91S_SYS, *AT91PS_SYS;




// -------- GPBR : (SYS Offset: 0x1290) GPBR General Purpose Register -------- 


// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR HSMC4 Chip Select interface
// *****************************************************************************

typedef struct _AT91S_HSMC4_CS {
	AT91_REG	 HSMC4_SETUP; 	// Setup Register
	AT91_REG	 HSMC4_PULSE; 	// Pulse Register
	AT91_REG	 HSMC4_CYCLE; 	// Cycle Register
	AT91_REG	 HSMC4_TIMINGS; 	// Timmings Register
	AT91_REG	 HSMC4_MODE; 	// Mode Register
} AT91S_HSMC4_CS, *AT91PS_HSMC4_CS;
#line 466 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_SETUP : (HSMC4_CS Offset: 0x0) HSMC4 SETUP -------- 




// -------- HSMC4_PULSE : (HSMC4_CS Offset: 0x4) HSMC4 PULSE -------- 




// -------- HSMC4_CYCLE : (HSMC4_CS Offset: 0x8) HSMC4 CYCLE -------- 


// -------- HSMC4_TIMINGS : (HSMC4_CS Offset: 0xc) HSMC4 TIMINGS -------- 
#line 488 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_MODE : (HSMC4_CS Offset: 0x10) HSMC4 MODE -------- 
#line 510 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR AHB Static Memory Controller 4 Interface
// *****************************************************************************

typedef struct _AT91S_HSMC4 {
	AT91_REG	 HSMC4_CFG; 	// Configuration Register
	AT91_REG	 HSMC4_CTRL; 	// Control Register
	AT91_REG	 HSMC4_SR; 	// Status Register
	AT91_REG	 HSMC4_IER; 	// Interrupt Enable Register
	AT91_REG	 HSMC4_IDR; 	// Interrupt Disable Register
	AT91_REG	 HSMC4_IMR; 	// Interrupt Mask Register
	AT91_REG	 HSMC4_ADDR; 	// Address Cycle Zero Register
	AT91_REG	 HSMC4_BANK; 	// Bank Register
	AT91_REG	 HSMC4_ECCCR; 	// ECC reset register
	AT91_REG	 HSMC4_ECCCMD; 	// ECC Page size register
	AT91_REG	 HSMC4_ECCSR1; 	// ECC Status register 1
	AT91_REG	 HSMC4_ECCPR0; 	// ECC Parity register 0
	AT91_REG	 HSMC4_ECCPR1; 	// ECC Parity register 1
	AT91_REG	 HSMC4_ECCSR2; 	// ECC Status register 2
	AT91_REG	 HSMC4_ECCPR2; 	// ECC Parity register 2
	AT91_REG	 HSMC4_ECCPR3; 	// ECC Parity register 3
	AT91_REG	 HSMC4_ECCPR4; 	// ECC Parity register 4
	AT91_REG	 HSMC4_ECCPR5; 	// ECC Parity register 5
	AT91_REG	 HSMC4_ECCPR6; 	// ECC Parity register 6
	AT91_REG	 HSMC4_ECCPR7; 	// ECC Parity register 7
	AT91_REG	 HSMC4_ECCPR8; 	// ECC Parity register 8
	AT91_REG	 HSMC4_ECCPR9; 	// ECC Parity register 9
	AT91_REG	 HSMC4_ECCPR10; 	// ECC Parity register 10
	AT91_REG	 HSMC4_ECCPR11; 	// ECC Parity register 11
	AT91_REG	 HSMC4_ECCPR12; 	// ECC Parity register 12
	AT91_REG	 HSMC4_ECCPR13; 	// ECC Parity register 13
	AT91_REG	 HSMC4_ECCPR14; 	// ECC Parity register 14
	AT91_REG	 HSMC4_Eccpr15; 	// ECC Parity register 15
	AT91_REG	 Reserved0[40]; 	// 
	AT91_REG	 HSMC4_OCMS; 	// OCMS MODE register
	AT91_REG	 HSMC4_KEY1; 	// KEY1 Register
	AT91_REG	 HSMC4_KEY2; 	// KEY2 Register
	AT91_REG	 Reserved1[50]; 	// 
	AT91_REG	 HSMC4_WPCR; 	// Write Protection Control register
	AT91_REG	 HSMC4_WPSR; 	// Write Protection Status Register
	AT91_REG	 HSMC4_ADDRSIZE; 	// Write Protection Status Register
	AT91_REG	 HSMC4_IPNAME1; 	// Write Protection Status Register
	AT91_REG	 HSMC4_IPNAME2; 	// Write Protection Status Register
	AT91_REG	 HSMC4_FEATURES; 	// Write Protection Status Register
	AT91_REG	 HSMC4_VER; 	// HSMC4 Version Register
	AT91_REG	 HSMC4_DUMMY; 	// This rtegister was created only ti have AHB constants
} AT91S_HSMC4, *AT91PS_HSMC4;
#line 600 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_CFG : (HSMC4 Offset: 0x0) Configuration Register -------- 
#line 620 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_CTRL : (HSMC4 Offset: 0x4) Control Register -------- 
#line 635 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_SR : (HSMC4 Offset: 0x8) HSMC4 Status Register -------- 
#line 665 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_IER : (HSMC4 Offset: 0xc) HSMC4 Interrupt Enable Register -------- 
// -------- HSMC4_IDR : (HSMC4 Offset: 0x10) HSMC4 Interrupt Disable Register -------- 
// -------- HSMC4_IMR : (HSMC4 Offset: 0x14) HSMC4 Interrupt Mask Register -------- 
// -------- HSMC4_ADDR : (HSMC4 Offset: 0x18) Address Cycle Zero Register -------- 

// -------- HSMC4_BANK : (HSMC4 Offset: 0x1c) Bank Register -------- 
#line 680 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_ECCCR : (HSMC4 Offset: 0x20) ECC Control Register -------- 

// -------- HSMC4_ECCCMD : (HSMC4 Offset: 0x24) ECC mode register -------- 
#line 692 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_ECCSR1 : (HSMC4 Offset: 0x28) ECC Status Register 1 -------- 
#line 717 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_ECCPR0 : (HSMC4 Offset: 0x2c) HSMC4 ECC parity Register 0 -------- 



// -------- HSMC4_ECCPR1 : (HSMC4 Offset: 0x30) HSMC4 ECC parity Register 1 -------- 
// -------- HSMC4_ECCSR2 : (HSMC4 Offset: 0x34) ECC Status Register 2 -------- 
#line 747 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_ECCPR2 : (HSMC4 Offset: 0x38) HSMC4 ECC parity Register 2 -------- 
// -------- HSMC4_ECCPR3 : (HSMC4 Offset: 0x3c) HSMC4 ECC parity Register 3 -------- 
// -------- HSMC4_ECCPR4 : (HSMC4 Offset: 0x40) HSMC4 ECC parity Register 4 -------- 
// -------- HSMC4_ECCPR5 : (HSMC4 Offset: 0x44) HSMC4 ECC parity Register 5 -------- 
// -------- HSMC4_ECCPR6 : (HSMC4 Offset: 0x48) HSMC4 ECC parity Register 6 -------- 
// -------- HSMC4_ECCPR7 : (HSMC4 Offset: 0x4c) HSMC4 ECC parity Register 7 -------- 
// -------- HSMC4_ECCPR8 : (HSMC4 Offset: 0x50) HSMC4 ECC parity Register 8 -------- 
// -------- HSMC4_ECCPR9 : (HSMC4 Offset: 0x54) HSMC4 ECC parity Register 9 -------- 
// -------- HSMC4_ECCPR10 : (HSMC4 Offset: 0x58) HSMC4 ECC parity Register 10 -------- 
// -------- HSMC4_ECCPR11 : (HSMC4 Offset: 0x5c) HSMC4 ECC parity Register 11 -------- 
// -------- HSMC4_ECCPR12 : (HSMC4 Offset: 0x60) HSMC4 ECC parity Register 12 -------- 
// -------- HSMC4_ECCPR13 : (HSMC4 Offset: 0x64) HSMC4 ECC parity Register 13 -------- 
// -------- HSMC4_ECCPR14 : (HSMC4 Offset: 0x68) HSMC4 ECC parity Register 14 -------- 
// -------- HSMC4_ECCPR15 : (HSMC4 Offset: 0x6c) HSMC4 ECC parity Register 15 -------- 
// -------- HSMC4_OCMS : (HSMC4 Offset: 0x110) HSMC4 OCMS Register -------- 


// -------- HSMC4_KEY1 : (HSMC4 Offset: 0x114) HSMC4 OCMS KEY1 Register -------- 

// -------- HSMC4_OCMS_KEY2 : (HSMC4 Offset: 0x118) HSMC4 OCMS KEY2 Register -------- 

// -------- HSMC4_WPCR : (HSMC4 Offset: 0x1e4) HSMC4 Witre Protection Control Register -------- 


// -------- HSMC4_WPSR : (HSMC4 Offset: 0x1e8) HSMC4 WPSR Register -------- 
#line 778 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HSMC4_VER : (HSMC4 Offset: 0x1fc) HSMC4 VERSION Register -------- 
// -------- HSMC4_DUMMY : (HSMC4 Offset: 0x200) HSMC4 DUMMY REGISTER -------- 
#line 802 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR AHB Matrix2 Interface
// *****************************************************************************

typedef struct _AT91S_HMATRIX2 {
	AT91_REG	 HMATRIX2_MCFG0; 	//  Master Configuration Register 0 : ARM I and D
	AT91_REG	 HMATRIX2_MCFG1; 	//  Master Configuration Register 1 : ARM S
	AT91_REG	 HMATRIX2_MCFG2; 	//  Master Configuration Register 2
	AT91_REG	 HMATRIX2_MCFG3; 	//  Master Configuration Register 3
	AT91_REG	 HMATRIX2_MCFG4; 	//  Master Configuration Register 4
	AT91_REG	 HMATRIX2_MCFG5; 	//  Master Configuration Register 5
	AT91_REG	 HMATRIX2_MCFG6; 	//  Master Configuration Register 6
	AT91_REG	 HMATRIX2_MCFG7; 	//  Master Configuration Register 7
	AT91_REG	 Reserved0[8]; 	// 
	AT91_REG	 HMATRIX2_SCFG0; 	//  Slave Configuration Register 0
	AT91_REG	 HMATRIX2_SCFG1; 	//  Slave Configuration Register 1
	AT91_REG	 HMATRIX2_SCFG2; 	//  Slave Configuration Register 2
	AT91_REG	 HMATRIX2_SCFG3; 	//  Slave Configuration Register 3
	AT91_REG	 HMATRIX2_SCFG4; 	//  Slave Configuration Register 4
	AT91_REG	 HMATRIX2_SCFG5; 	//  Slave Configuration Register 5
	AT91_REG	 HMATRIX2_SCFG6; 	//  Slave Configuration Register 6
	AT91_REG	 HMATRIX2_SCFG7; 	//  Slave Configuration Register 5
	AT91_REG	 HMATRIX2_SCFG8; 	//  Slave Configuration Register 8
	AT91_REG	 HMATRIX2_SCFG9; 	//  Slave Configuration Register 9
	AT91_REG	 Reserved1[42]; 	// 
	AT91_REG	 HMATRIX2_SFR0 ; 	//  Special Function Register 0
	AT91_REG	 HMATRIX2_SFR1 ; 	//  Special Function Register 1
	AT91_REG	 HMATRIX2_SFR2 ; 	//  Special Function Register 2
	AT91_REG	 HMATRIX2_SFR3 ; 	//  Special Function Register 3
	AT91_REG	 HMATRIX2_SFR4 ; 	//  Special Function Register 4
	AT91_REG	 HMATRIX2_SFR5 ; 	//  Special Function Register 5
	AT91_REG	 HMATRIX2_SFR6 ; 	//  Special Function Register 6
	AT91_REG	 HMATRIX2_SFR7 ; 	//  Special Function Register 7
	AT91_REG	 HMATRIX2_SFR8 ; 	//  Special Function Register 8
	AT91_REG	 HMATRIX2_SFR9 ; 	//  Special Function Register 9
	AT91_REG	 HMATRIX2_SFR10; 	//  Special Function Register 10
	AT91_REG	 HMATRIX2_SFR11; 	//  Special Function Register 11
	AT91_REG	 HMATRIX2_SFR12; 	//  Special Function Register 12
	AT91_REG	 HMATRIX2_SFR13; 	//  Special Function Register 13
	AT91_REG	 HMATRIX2_SFR14; 	//  Special Function Register 14
	AT91_REG	 HMATRIX2_SFR15; 	//  Special Function Register 15
	AT91_REG	 Reserved2[39]; 	// 
	AT91_REG	 HMATRIX2_ADDRSIZE; 	// HMATRIX2 ADDRSIZE REGISTER 
	AT91_REG	 HMATRIX2_IPNAME1; 	// HMATRIX2 IPNAME1 REGISTER 
	AT91_REG	 HMATRIX2_IPNAME2; 	// HMATRIX2 IPNAME2 REGISTER 
	AT91_REG	 HMATRIX2_FEATURES; 	// HMATRIX2 FEATURES REGISTER 
	AT91_REG	 HMATRIX2_VER; 	// HMATRIX2 VERSION REGISTER 
} AT91S_HMATRIX2, *AT91PS_HMATRIX2;
#line 893 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MATRIX_MCFG0 : (HMATRIX2 Offset: 0x0) Master Configuration Register ARM bus I and D -------- 
#line 903 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MATRIX_MCFG1 : (HMATRIX2 Offset: 0x4) Master Configuration Register ARM bus S -------- 
// -------- MATRIX_MCFG2 : (HMATRIX2 Offset: 0x8) Master Configuration Register -------- 
// -------- MATRIX_MCFG3 : (HMATRIX2 Offset: 0xc) Master Configuration Register -------- 
// -------- MATRIX_MCFG4 : (HMATRIX2 Offset: 0x10) Master Configuration Register -------- 
// -------- MATRIX_MCFG5 : (HMATRIX2 Offset: 0x14) Master Configuration Register -------- 
// -------- MATRIX_MCFG6 : (HMATRIX2 Offset: 0x18) Master Configuration Register -------- 
// -------- MATRIX_MCFG7 : (HMATRIX2 Offset: 0x1c) Master Configuration Register -------- 
// -------- MATRIX_SCFG0 : (HMATRIX2 Offset: 0x40) Slave Configuration Register 0 -------- 
#line 918 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MATRIX_SCFG1 : (HMATRIX2 Offset: 0x44) Slave Configuration Register 1 -------- 


// -------- MATRIX_SCFG2 : (HMATRIX2 Offset: 0x48) Slave Configuration Register 2 -------- 


// -------- MATRIX_SCFG3 : (HMATRIX2 Offset: 0x4c) Slave Configuration Register 3 -------- 


// -------- MATRIX_SCFG4 : (HMATRIX2 Offset: 0x50) Slave Configuration Register 4 -------- 


// -------- MATRIX_SCFG5 : (HMATRIX2 Offset: 0x54) Slave Configuration Register 5 -------- 


// -------- MATRIX_SCFG6 : (HMATRIX2 Offset: 0x58) Slave Configuration Register 6 -------- 


// -------- MATRIX_SCFG7 : (HMATRIX2 Offset: 0x5c) Slave Configuration Register 7 -------- 


// -------- MATRIX_SCFG8 : (HMATRIX2 Offset: 0x60) Slave Configuration Register 8 -------- 



// -------- MATRIX_SCFG9 : (HMATRIX2 Offset: 0x64) Slave Configuration Register 9 -------- 



// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x110) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x114) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x118) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x11c) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x120) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x124) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x128) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x12c) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x130) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x134) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x138) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x13c) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x140) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x144) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x148) Special Function Register 0 -------- 
// -------- MATRIX_SFR0 : (HMATRIX2 Offset: 0x14c) Special Function Register 0 -------- 
// -------- HMATRIX2_VER : (HMATRIX2 Offset: 0x1fc)  VERSION  Register -------- 


// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR NESTED vector Interrupt Controller
// *****************************************************************************

typedef struct _AT91S_NVIC {
	AT91_REG	 Reserved0[1]; 	// 
	AT91_REG	 NVIC_ICTR; 	// Interrupt Control Type Register
	AT91_REG	 Reserved1[2]; 	// 
	AT91_REG	 NVIC_STICKCSR; 	// SysTick Control and Status Register
	AT91_REG	 NVIC_STICKRVR; 	// SysTick Reload Value Register
	AT91_REG	 NVIC_STICKCVR; 	// SysTick Current Value Register
	AT91_REG	 NVIC_STICKCALVR; 	// SysTick Calibration Value Register
	AT91_REG	 Reserved2[56]; 	// 
	AT91_REG	 NVIC_ISER[8]; 	// Set Enable Register
	AT91_REG	 Reserved3[24]; 	// 
	AT91_REG	 NVIC_ICER[8]; 	// Clear enable Register
	AT91_REG	 Reserved4[24]; 	// 
	AT91_REG	 NVIC_ISPR[8]; 	// Set Pending Register
	AT91_REG	 Reserved5[24]; 	// 
	AT91_REG	 NVIC_ICPR[8]; 	// Clear Pending Register
	AT91_REG	 Reserved6[24]; 	// 
	AT91_REG	 NVIC_ABR[8]; 	// Active Bit Register
	AT91_REG	 Reserved7[56]; 	// 
	AT91_REG	 NVIC_IPR[60]; 	// Interrupt Mask Register
	AT91_REG	 Reserved8[516]; 	// 
	AT91_REG	 NVIC_CPUID; 	// CPUID Base Register
	AT91_REG	 NVIC_ICSR; 	// Interrupt Control State Register
	AT91_REG	 NVIC_VTOFFR; 	// Vector Table Offset Register
	AT91_REG	 NVIC_AIRCR; 	// Application Interrupt/Reset Control Reg
	AT91_REG	 NVIC_SCR; 	// System Control Register
	AT91_REG	 NVIC_CCR; 	// Configuration Control Register
	AT91_REG	 NVIC_HAND4PR; 	// System Handlers 4-7 Priority Register
	AT91_REG	 NVIC_HAND8PR; 	// System Handlers 8-11 Priority Register
	AT91_REG	 NVIC_HAND12PR; 	// System Handlers 12-15 Priority Register
	AT91_REG	 NVIC_HANDCSR; 	// System Handler Control and State Register
	AT91_REG	 NVIC_CFSR; 	// Configurable Fault Status Register
	AT91_REG	 NVIC_HFSR; 	// Hard Fault Status Register
	AT91_REG	 NVIC_DFSR; 	// Debug Fault Status Register
	AT91_REG	 NVIC_MMAR; 	// Mem Manage Address Register
	AT91_REG	 NVIC_BFAR; 	// Bus Fault Address Register
	AT91_REG	 NVIC_AFSR; 	// Auxiliary Fault Status Register
	AT91_REG	 NVIC_PFR0; 	// Processor Feature register0
	AT91_REG	 NVIC_PFR1; 	// Processor Feature register1
	AT91_REG	 NVIC_DFR0; 	// Debug Feature register0
	AT91_REG	 NVIC_AFR0; 	// Auxiliary Feature register0
	AT91_REG	 NVIC_MMFR0; 	// Memory Model Feature register0
	AT91_REG	 NVIC_MMFR1; 	// Memory Model Feature register1
	AT91_REG	 NVIC_MMFR2; 	// Memory Model Feature register2
	AT91_REG	 NVIC_MMFR3; 	// Memory Model Feature register3
	AT91_REG	 NVIC_ISAR0; 	// ISA Feature register0
	AT91_REG	 NVIC_ISAR1; 	// ISA Feature register1
	AT91_REG	 NVIC_ISAR2; 	// ISA Feature register2
	AT91_REG	 NVIC_ISAR3; 	// ISA Feature register3
	AT91_REG	 NVIC_ISAR4; 	// ISA Feature register4
	AT91_REG	 Reserved9[99]; 	// 
	AT91_REG	 NVIC_STIR; 	// Software Trigger Interrupt Register
	AT91_REG	 Reserved10[51]; 	// 
	AT91_REG	 NVIC_PID4; 	// Peripheral identification register
	AT91_REG	 NVIC_PID5; 	// Peripheral identification register
	AT91_REG	 NVIC_PID6; 	// Peripheral identification register
	AT91_REG	 NVIC_PID7; 	// Peripheral identification register
	AT91_REG	 NVIC_PID0; 	// Peripheral identification register b7:0
	AT91_REG	 NVIC_PID1; 	// Peripheral identification register b15:8
	AT91_REG	 NVIC_PID2; 	// Peripheral identification register b23:16
	AT91_REG	 NVIC_PID3; 	// Peripheral identification register b31:24
	AT91_REG	 NVIC_CID0; 	// Component identification register b7:0
	AT91_REG	 NVIC_CID1; 	// Component identification register b15:8
	AT91_REG	 NVIC_CID2; 	// Component identification register b23:16
	AT91_REG	 NVIC_CID3; 	// Component identification register b31:24
} AT91S_NVIC, *AT91PS_NVIC;
#line 1092 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- NVIC_ICTR : (NVIC Offset: 0x4) Interrupt Controller Type Register -------- 
#line 1110 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- NVIC_STICKCSR : (NVIC Offset: 0x10) SysTick Control and Status Register -------- 




// -------- NVIC_STICKRVR : (NVIC Offset: 0x14) SysTick Reload Value Register -------- 

// -------- NVIC_STICKCVR : (NVIC Offset: 0x18) SysTick Current Value Register -------- 

// -------- NVIC_STICKCALVR : (NVIC Offset: 0x1c) SysTick Calibration Value Register -------- 



// -------- NVIC_IPR : (NVIC Offset: 0x400) Interrupt Priority Registers -------- 




// -------- NVIC_CPUID : (NVIC Offset: 0xd00) CPU ID Base Register -------- 





// -------- NVIC_ICSR : (NVIC Offset: 0xd04) Interrupt Control State Register -------- 
#line 1145 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- NVIC_VTOFFR : (NVIC Offset: 0xd08) Vector Table Offset Register -------- 




// -------- NVIC_AIRCR : (NVIC Offset: 0xd0c) Application Interrupt and Reset Control Register -------- 
#line 1162 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- NVIC_SCR : (NVIC Offset: 0xd10) System Control Register -------- 



// -------- NVIC_CCR : (NVIC Offset: 0xd14) Configuration Control Register -------- 
#line 1173 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- NVIC_HAND4PR : (NVIC Offset: 0xd18) System Handlers 4-7 Priority Register -------- 




// -------- NVIC_HAND8PR : (NVIC Offset: 0xd1c) System Handlers 8-11 Priority Register -------- 




// -------- NVIC_HAND12PR : (NVIC Offset: 0xd20) System Handlers 12-15 Priority Register -------- 




// -------- NVIC_HANDCSR : (NVIC Offset: 0xd24) System Handler Control and State Register -------- 
#line 1203 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- NVIC_CFSR : (NVIC Offset: 0xd28) Configurable Fault Status Registers -------- 



// -------- NVIC_BFAR : (NVIC Offset: 0xd38) Bus Fault Address Register -------- 
#line 1214 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- NVIC_PFR0 : (NVIC Offset: 0xd40) Processor Feature register0 (ID_PFR0) -------- 


// -------- NVIC_PFR1 : (NVIC Offset: 0xd44) Processor Feature register1 (ID_PFR1) -------- 

// -------- NVIC_DFR0 : (NVIC Offset: 0xd48) Debug Feature register0 (ID_DFR0) -------- 

// -------- NVIC_MMFR0 : (NVIC Offset: 0xd50) Memory Model Feature register0 (ID_MMFR0) -------- 



// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR NESTED vector Interrupt Controller
// *****************************************************************************

typedef struct _AT91S_MPU {
	AT91_REG	 MPU_TYPE; 	// MPU Type Register
	AT91_REG	 MPU_CTRL; 	// MPU Control Register
	AT91_REG	 MPU_REG_NB; 	// MPU Region Number Register
	AT91_REG	 MPU_REG_BASE_ADDR; 	// MPU Region Base Address Register
	AT91_REG	 MPU_ATTR_SIZE; 	// MPU  Attribute and Size Register
	AT91_REG	 MPU_REG_BASE_ADDR1; 	// MPU Region Base Address Register alias 1
	AT91_REG	 MPU_ATTR_SIZE1; 	// MPU  Attribute and Size Register alias 1
	AT91_REG	 MPU_REG_BASE_ADDR2; 	// MPU Region Base Address Register alias 2
	AT91_REG	 MPU_ATTR_SIZE2; 	// MPU  Attribute and Size Register alias 2
	AT91_REG	 MPU_REG_BASE_ADDR3; 	// MPU Region Base Address Register alias 3
	AT91_REG	 MPU_ATTR_SIZE3; 	// MPU  Attribute and Size Register alias 3
} AT91S_MPU, *AT91PS_MPU;
#line 1256 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MPU_TYPE : (MPU Offset: 0x0)  -------- 



// -------- MPU_CTRL : (MPU Offset: 0x4)  -------- 



// -------- MPU_REG_NB : (MPU Offset: 0x8)  -------- 

// -------- MPU_REG_BASE_ADDR : (MPU Offset: 0xc)  -------- 



// -------- MPU_ATTR_SIZE : (MPU Offset: 0x10)  -------- 
#line 1280 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR CORTEX_M3 Registers
// *****************************************************************************

typedef struct _AT91S_CM3 {
	AT91_REG	 CM3_CPUID; 	// CPU ID Base Register
	AT91_REG	 CM3_ICSR; 	// Interrupt Control State Register
	AT91_REG	 CM3_VTOR; 	// Vector Table Offset Register
	AT91_REG	 CM3_AIRCR; 	// Application Interrupt and Reset Control Register
	AT91_REG	 CM3_SCR; 	// System Controller Register
	AT91_REG	 CM3_CCR; 	// Configuration Control Register
	AT91_REG	 CM3_SHPR[3]; 	// System Handler Priority Register
	AT91_REG	 CM3_SHCSR; 	// System Handler Control and State Register
} AT91S_CM3, *AT91PS_CM3;
#line 1306 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- CM3_CPUID : (CM3 Offset: 0x0)  -------- 
// -------- CM3_AIRCR : (CM3 Offset: 0xc)  -------- 

// -------- CM3_SCR : (CM3 Offset: 0x10)  -------- 



// -------- CM3_SHCSR : (CM3 Offset: 0x24)  -------- 


// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Peripheral DMA Controller
// *****************************************************************************

typedef struct _AT91S_PDC {
	AT91_REG	 PDC_RPR; 	// Receive Pointer Register
	AT91_REG	 PDC_RCR; 	// Receive Counter Register
	AT91_REG	 PDC_TPR; 	// Transmit Pointer Register
	AT91_REG	 PDC_TCR; 	// Transmit Counter Register
	AT91_REG	 PDC_RNPR; 	// Receive Next Pointer Register
	AT91_REG	 PDC_RNCR; 	// Receive Next Counter Register
	AT91_REG	 PDC_TNPR; 	// Transmit Next Pointer Register
	AT91_REG	 PDC_TNCR; 	// Transmit Next Counter Register
	AT91_REG	 PDC_PTCR; 	// PDC Transfer Control Register
	AT91_REG	 PDC_PTSR; 	// PDC Transfer Status Register
} AT91S_PDC, *AT91PS_PDC;
#line 1345 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PDC_PTCR : (PDC Offset: 0x20) PDC Transfer Control Register -------- 




// -------- PDC_PTSR : (PDC Offset: 0x24) PDC Transfer Status Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Debug Unit
// *****************************************************************************

typedef struct _AT91S_DBGU {
	AT91_REG	 DBGU_CR; 	// Control Register
	AT91_REG	 DBGU_MR; 	// Mode Register
	AT91_REG	 DBGU_IER; 	// Interrupt Enable Register
	AT91_REG	 DBGU_IDR; 	// Interrupt Disable Register
	AT91_REG	 DBGU_IMR; 	// Interrupt Mask Register
	AT91_REG	 DBGU_CSR; 	// Channel Status Register
	AT91_REG	 DBGU_RHR; 	// Receiver Holding Register
	AT91_REG	 DBGU_THR; 	// Transmitter Holding Register
	AT91_REG	 DBGU_BRGR; 	// Baud Rate Generator Register
	AT91_REG	 Reserved0[9]; 	// 
	AT91_REG	 DBGU_FNTR; 	// Force NTRST Register
	AT91_REG	 Reserved1[40]; 	// 
	AT91_REG	 DBGU_ADDRSIZE; 	// DBGU ADDRSIZE REGISTER 
	AT91_REG	 DBGU_IPNAME1; 	// DBGU IPNAME1 REGISTER 
	AT91_REG	 DBGU_IPNAME2; 	// DBGU IPNAME2 REGISTER 
	AT91_REG	 DBGU_FEATURES; 	// DBGU FEATURES REGISTER 
	AT91_REG	 DBGU_VER; 	// DBGU VERSION REGISTER 
	AT91_REG	 DBGU_RPR; 	// Receive Pointer Register
	AT91_REG	 DBGU_RCR; 	// Receive Counter Register
	AT91_REG	 DBGU_TPR; 	// Transmit Pointer Register
	AT91_REG	 DBGU_TCR; 	// Transmit Counter Register
	AT91_REG	 DBGU_RNPR; 	// Receive Next Pointer Register
	AT91_REG	 DBGU_RNCR; 	// Receive Next Counter Register
	AT91_REG	 DBGU_TNPR; 	// Transmit Next Pointer Register
	AT91_REG	 DBGU_TNCR; 	// Transmit Next Counter Register
	AT91_REG	 DBGU_PTCR; 	// PDC Transfer Control Register
	AT91_REG	 DBGU_PTSR; 	// PDC Transfer Status Register
	AT91_REG	 Reserved2[6]; 	// 
	AT91_REG	 DBGU_CIDR; 	// Chip ID Register
	AT91_REG	 DBGU_EXID; 	// Chip ID Extension Register
} AT91S_DBGU, *AT91PS_DBGU;
#line 1408 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- DBGU_CR : (DBGU Offset: 0x0) Debug Unit Control Register -------- 
#line 1416 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- DBGU_MR : (DBGU Offset: 0x4) Debug Unit Mode Register -------- 
#line 1428 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- DBGU_IER : (DBGU Offset: 0x8) Debug Unit Interrupt Enable Register -------- 
#line 1441 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- DBGU_IDR : (DBGU Offset: 0xc) Debug Unit Interrupt Disable Register -------- 
// -------- DBGU_IMR : (DBGU Offset: 0x10) Debug Unit Interrupt Mask Register -------- 
// -------- DBGU_CSR : (DBGU Offset: 0x14) Debug Unit Channel Status Register -------- 
// -------- DBGU_FNTR : (DBGU Offset: 0x48) Debug Unit FORCE_NTRST Register -------- 


// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Parallel Input Output Controler
// *****************************************************************************

typedef struct _AT91S_PIO {
	AT91_REG	 PIO_PER; 	// PIO Enable Register
	AT91_REG	 PIO_PDR; 	// PIO Disable Register
	AT91_REG	 PIO_PSR; 	// PIO Status Register
	AT91_REG	 Reserved0[1]; 	// 
	AT91_REG	 PIO_OER; 	// Output Enable Register
	AT91_REG	 PIO_ODR; 	// Output Disable Registerr
	AT91_REG	 PIO_OSR; 	// Output Status Register
	AT91_REG	 Reserved1[1]; 	// 
	AT91_REG	 PIO_IFER; 	// Input Filter Enable Register
	AT91_REG	 PIO_IFDR; 	// Input Filter Disable Register
	AT91_REG	 PIO_IFSR; 	// Input Filter Status Register
	AT91_REG	 Reserved2[1]; 	// 
	AT91_REG	 PIO_SODR; 	// Set Output Data Register
	AT91_REG	 PIO_CODR; 	// Clear Output Data Register
	AT91_REG	 PIO_ODSR; 	// Output Data Status Register
	AT91_REG	 PIO_PDSR; 	// Pin Data Status Register
	AT91_REG	 PIO_IER; 	// Interrupt Enable Register
	AT91_REG	 PIO_IDR; 	// Interrupt Disable Register
	AT91_REG	 PIO_IMR; 	// Interrupt Mask Register
	AT91_REG	 PIO_ISR; 	// Interrupt Status Register
	AT91_REG	 PIO_MDER; 	// Multi-driver Enable Register
	AT91_REG	 PIO_MDDR; 	// Multi-driver Disable Register
	AT91_REG	 PIO_MDSR; 	// Multi-driver Status Register
	AT91_REG	 Reserved3[1]; 	// 
	AT91_REG	 PIO_PPUDR; 	// Pull-up Disable Register
	AT91_REG	 PIO_PPUER; 	// Pull-up Enable Register
	AT91_REG	 PIO_PPUSR; 	// Pull-up Status Register
	AT91_REG	 Reserved4[1]; 	// 
	AT91_REG	 PIO_ABSR; 	// Peripheral AB Select Register
	AT91_REG	 Reserved5[3]; 	// 
	AT91_REG	 PIO_SCIFSR; 	// System Clock Glitch Input Filter Select Register
	AT91_REG	 PIO_DIFSR; 	// Debouncing Input Filter Select Register
	AT91_REG	 PIO_IFDGSR; 	// Glitch or Debouncing Input Filter Clock Selection Status Register
	AT91_REG	 PIO_SCDR; 	// Slow Clock Divider Debouncing Register
	AT91_REG	 Reserved6[4]; 	// 
	AT91_REG	 PIO_OWER; 	// Output Write Enable Register
	AT91_REG	 PIO_OWDR; 	// Output Write Disable Register
	AT91_REG	 PIO_OWSR; 	// Output Write Status Register
	AT91_REG	 Reserved7[1]; 	// 
	AT91_REG	 PIO_AIMER; 	// Additional Interrupt Modes Enable Register
	AT91_REG	 PIO_AIMDR; 	// Additional Interrupt Modes Disables Register
	AT91_REG	 PIO_AIMMR; 	// Additional Interrupt Modes Mask Register
	AT91_REG	 Reserved8[1]; 	// 
	AT91_REG	 PIO_ESR; 	// Edge Select Register
	AT91_REG	 PIO_LSR; 	// Level Select Register
	AT91_REG	 PIO_ELSR; 	// Edge/Level Status Register
	AT91_REG	 Reserved9[1]; 	// 
	AT91_REG	 PIO_FELLSR; 	// Falling Edge/Low Level Select Register
	AT91_REG	 PIO_REHLSR; 	// Rising Edge/ High Level Select Register
	AT91_REG	 PIO_FRLHSR; 	// Fall/Rise - Low/High Status Register
	AT91_REG	 Reserved10[1]; 	// 
	AT91_REG	 PIO_LOCKSR; 	// Lock Status Register
	AT91_REG	 Reserved11[6]; 	// 
	AT91_REG	 PIO_VER; 	// PIO VERSION REGISTER 
	AT91_REG	 Reserved12[8]; 	// 
	AT91_REG	 PIO_KER; 	// Keypad Controller Enable Register
	AT91_REG	 PIO_KRCR; 	// Keypad Controller Row Column Register
	AT91_REG	 PIO_KDR; 	// Keypad Controller Debouncing Register
	AT91_REG	 Reserved13[1]; 	// 
	AT91_REG	 PIO_KIER; 	// Keypad Controller Interrupt Enable Register
	AT91_REG	 PIO_KIDR; 	// Keypad Controller Interrupt Disable Register
	AT91_REG	 PIO_KIMR; 	// Keypad Controller Interrupt Mask Register
	AT91_REG	 PIO_KSR; 	// Keypad Controller Status Register
	AT91_REG	 PIO_KKPR; 	// Keypad Controller Key Press Register
	AT91_REG	 PIO_KKRR; 	// Keypad Controller Key Release Register
} AT91S_PIO, *AT91PS_PIO;
#line 1572 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PIO_KER : (PIO Offset: 0x120) Keypad Controller Enable Register -------- 

// -------- PIO_KRCR : (PIO Offset: 0x124) Keypad Controller Row Column Register -------- 


// -------- PIO_KDR : (PIO Offset: 0x128) Keypad Controller Debouncing Register -------- 

// -------- PIO_KIER : (PIO Offset: 0x130) Keypad Controller Interrupt Enable Register -------- 


// -------- PIO_KIDR : (PIO Offset: 0x134) Keypad Controller Interrupt Disable Register -------- 
// -------- PIO_KIMR : (PIO Offset: 0x138) Keypad Controller Interrupt Mask Register -------- 
// -------- PIO_KSR : (PIO Offset: 0x13c) Keypad Controller Status Register -------- 


// -------- PIO_KKPR : (PIO Offset: 0x140) Keypad Controller Key Press Register -------- 
#line 1596 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PIO_KKRR : (PIO Offset: 0x144) Keypad Controller Key Release Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Power Management Controler
// *****************************************************************************

typedef struct _AT91S_PMC {
	AT91_REG	 PMC_SCER; 	// System Clock Enable Register
	AT91_REG	 PMC_SCDR; 	// System Clock Disable Register
	AT91_REG	 PMC_SCSR; 	// System Clock Status Register
	AT91_REG	 Reserved0[1]; 	// 
	AT91_REG	 PMC_PCER; 	// Peripheral Clock Enable Register
	AT91_REG	 PMC_PCDR; 	// Peripheral Clock Disable Register
	AT91_REG	 PMC_PCSR; 	// Peripheral Clock Status Register
	AT91_REG	 PMC_UCKR; 	// UTMI Clock Configuration Register
	AT91_REG	 PMC_MOR; 	// Main Oscillator Register
	AT91_REG	 PMC_MCFR; 	// Main Clock  Frequency Register
	AT91_REG	 PMC_PLLAR; 	// PLL Register
	AT91_REG	 Reserved1[1]; 	// 
	AT91_REG	 PMC_MCKR; 	// Master Clock Register
	AT91_REG	 Reserved2[3]; 	// 
	AT91_REG	 PMC_PCKR[8]; 	// Programmable Clock Register
	AT91_REG	 PMC_IER; 	// Interrupt Enable Register
	AT91_REG	 PMC_IDR; 	// Interrupt Disable Register
	AT91_REG	 PMC_SR; 	// Status Register
	AT91_REG	 PMC_IMR; 	// Interrupt Mask Register
	AT91_REG	 PMC_FSMR; 	// Fast Startup Mode Register
	AT91_REG	 PMC_FSPR; 	// Fast Startup Polarity Register
	AT91_REG	 PMC_FOCR; 	// Fault Output Clear Register
	AT91_REG	 Reserved3[28]; 	// 
	AT91_REG	 PMC_ADDRSIZE; 	// PMC ADDRSIZE REGISTER 
	AT91_REG	 PMC_IPNAME1; 	// PMC IPNAME1 REGISTER 
	AT91_REG	 PMC_IPNAME2; 	// PMC IPNAME2 REGISTER 
	AT91_REG	 PMC_FEATURES; 	// PMC FEATURES REGISTER 
	AT91_REG	 PMC_VER; 	// APMC VERSION REGISTER
} AT91S_PMC, *AT91PS_PMC;
#line 1659 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PMC_SCER : (PMC Offset: 0x0) System Clock Enable Register -------- 




// -------- PMC_SCDR : (PMC Offset: 0x4) System Clock Disable Register -------- 
// -------- PMC_SCSR : (PMC Offset: 0x8) System Clock Status Register -------- 
// -------- CKGR_UCKR : (PMC Offset: 0x1c) UTMI Clock Configuration Register -------- 
#line 1675 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- CKGR_MOR : (PMC Offset: 0x20) Main Oscillator Register -------- 
#line 1685 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- CKGR_MCFR : (PMC Offset: 0x24) Main Clock Frequency Register -------- 


// -------- CKGR_PLLAR : (PMC Offset: 0x28) PLL A Register -------- 
#line 1700 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PMC_MCKR : (PMC Offset: 0x30) Master Clock Register -------- 
#line 1716 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PMC_PCKR : (PMC Offset: 0x40) Programmable Clock Register -------- 
// -------- PMC_IER : (PMC Offset: 0x60) PMC Interrupt Enable Register -------- 
#line 1728 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PMC_IDR : (PMC Offset: 0x64) PMC Interrupt Disable Register -------- 
// -------- PMC_SR : (PMC Offset: 0x68) PMC Status Register -------- 



// -------- PMC_IMR : (PMC Offset: 0x6c) PMC Interrupt Mask Register -------- 
// -------- PMC_FSMR : (PMC Offset: 0x70) Fast Startup Mode Register -------- 





// -------- PMC_FSPR : (PMC Offset: 0x74) Fast Startup Polarity Register -------- 

// -------- PMC_FOCR : (PMC Offset: 0x78) Fault Output Clear Register -------- 


// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Clock Generator Controler
// *****************************************************************************

typedef struct _AT91S_CKGR {
	AT91_REG	 CKGR_UCKR; 	// UTMI Clock Configuration Register
	AT91_REG	 CKGR_MOR; 	// Main Oscillator Register
	AT91_REG	 CKGR_MCFR; 	// Main Clock  Frequency Register
	AT91_REG	 CKGR_PLLAR; 	// PLL Register
} AT91S_CKGR, *AT91PS_CKGR;



// -------- CKGR_UCKR : (CKGR Offset: 0x0) UTMI Clock Configuration Register -------- 
// -------- CKGR_MOR : (CKGR Offset: 0x4) Main Oscillator Register -------- 
// -------- CKGR_MCFR : (CKGR Offset: 0x8) Main Clock Frequency Register -------- 
// -------- CKGR_PLLAR : (CKGR Offset: 0xc) PLL A Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Reset Controller Interface
// *****************************************************************************

typedef struct _AT91S_RSTC {
	AT91_REG	 RSTC_RCR; 	// Reset Control Register
	AT91_REG	 RSTC_RSR; 	// Reset Status Register
	AT91_REG	 RSTC_RMR; 	// Reset Mode Register
	AT91_REG	 Reserved0[60]; 	// 
	AT91_REG	 RSTC_VER; 	// Version Register
} AT91S_RSTC, *AT91PS_RSTC;
#line 1781 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- RSTC_RCR : (RSTC Offset: 0x0) Reset Control Register -------- 





// -------- RSTC_RSR : (RSTC Offset: 0x4) Reset Status Register -------- 
#line 1797 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- RSTC_RMR : (RSTC Offset: 0x8) Reset Mode Register -------- 




// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Supply Controller Interface
// *****************************************************************************

typedef struct _AT91S_SUPC {
  AT91_REG SUPC_CR;   // Supply Controller Control Register
  AT91_REG SUPC_SMMR; // Supply Controller Supply Monitor Mode Register
  AT91_REG SUPC_MR;   // Supply Controller Mode Register
  AT91_REG SUPC_WUMR; // Supply Controller Wake Up Mode Register
  AT91_REG SUPC_WUIR; // Supply Controller Wake Up Inputs Register
  AT91_REG SUPC_SR;   // Supply Controller Status Register
} AT91S_SUPC, *AT91PS_SUPC;
#line 1822 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SUPC_CR : (SUPC Offset: 0x00) Supply Controller Control Register --------
#line 1830 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SUPC_SMMR : (SUPC Offset: 0x04) Supply Controller Supply Monitor Mode Register --------
#line 1860 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SUPC_MR : (SUPC Offset: 0x08) Supply Controller Mode Register --------
#line 1874 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SUPC_WUMR : (SUPC Offset: 0x0C) Supply Controller Wake Up Mode Register --------
#line 1901 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SUPC_WUIR : (SUPC Offset: 0x10) Supply Controller Wake Up Inputs Register --------
#line 1998 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SUPC_SR : (SUPC Offset: 0x14) Supply Controller Status Register --------
#line 2074 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Real Time Timer Controller Interface
// *****************************************************************************

typedef struct _AT91S_RTTC {
	AT91_REG	 RTTC_RTMR; 	// Real-time Mode Register
	AT91_REG	 RTTC_RTAR; 	// Real-time Alarm Register
	AT91_REG	 RTTC_RTVR; 	// Real-time Value Register
	AT91_REG	 RTTC_RTSR; 	// Real-time Status Register
} AT91S_RTTC, *AT91PS_RTTC;
#line 2092 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- RTTC_RTMR : (RTTC Offset: 0x0) Real-time Mode Register -------- 




// -------- RTTC_RTAR : (RTTC Offset: 0x4) Real-time Alarm Register -------- 

// -------- RTTC_RTVR : (RTTC Offset: 0x8) Current Real-time Value Register -------- 

// -------- RTTC_RTSR : (RTTC Offset: 0xc) Real-time Status Register -------- 



// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Watchdog Timer Controller Interface
// *****************************************************************************

typedef struct _AT91S_WDTC {
	AT91_REG	 WDTC_WDCR; 	// Watchdog Control Register
	AT91_REG	 WDTC_WDMR; 	// Watchdog Mode Register
	AT91_REG	 WDTC_WDSR; 	// Watchdog Status Register
} AT91S_WDTC, *AT91PS_WDTC;
#line 2120 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- WDTC_WDCR : (WDTC Offset: 0x0) Periodic Interval Image Register -------- 


// -------- WDTC_WDMR : (WDTC Offset: 0x4) Watchdog Mode Register -------- 
#line 2132 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- WDTC_WDSR : (WDTC Offset: 0x8) Watchdog Status Register -------- 



// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Real-time Clock Alarm and Parallel Load Interface
// *****************************************************************************

typedef struct _AT91S_RTC {
	AT91_REG	 RTC_CR; 	// Control Register
	AT91_REG	 RTC_MR; 	// Mode Register
	AT91_REG	 RTC_TIMR; 	// Time Register
	AT91_REG	 RTC_CALR; 	// Calendar Register
	AT91_REG	 RTC_TIMALR; 	// Time Alarm Register
	AT91_REG	 RTC_CALALR; 	// Calendar Alarm Register
	AT91_REG	 RTC_SR; 	// Status Register
	AT91_REG	 RTC_SCCR; 	// Status Clear Command Register
	AT91_REG	 RTC_IER; 	// Interrupt Enable Register
	AT91_REG	 RTC_IDR; 	// Interrupt Disable Register
	AT91_REG	 RTC_IMR; 	// Interrupt Mask Register
	AT91_REG	 RTC_VER; 	// Valid Entry Register
} AT91S_RTC, *AT91PS_RTC;
#line 2169 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- RTC_CR : (RTC Offset: 0x0) RTC Control Register -------- 
#line 2181 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- RTC_MR : (RTC Offset: 0x4) RTC Mode Register -------- 

// -------- RTC_TIMR : (RTC Offset: 0x8) RTC Time Register -------- 




// -------- RTC_CALR : (RTC Offset: 0xc) RTC Calendar Register -------- 





// -------- RTC_TIMALR : (RTC Offset: 0x10) RTC Time Alarm Register -------- 



// -------- RTC_CALALR : (RTC Offset: 0x14) RTC Calendar Alarm Register -------- 


// -------- RTC_SR : (RTC Offset: 0x18) RTC Status Register -------- 





// -------- RTC_SCCR : (RTC Offset: 0x1c) RTC Status Clear Command Register -------- 
// -------- RTC_IER : (RTC Offset: 0x20) RTC Interrupt Enable Register -------- 
// -------- RTC_IDR : (RTC Offset: 0x24) RTC Interrupt Disable Register -------- 
// -------- RTC_IMR : (RTC Offset: 0x28) RTC Interrupt Mask Register -------- 
// -------- RTC_VER : (RTC Offset: 0x2c) RTC Valid Entry Register -------- 





// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Analog to Digital Convertor
// *****************************************************************************

typedef struct _AT91S_ADC {
	AT91_REG	 ADC_CR; 	// ADC Control Register
	AT91_REG	 ADC_MR; 	// ADC Mode Register
	AT91_REG	 Reserved0[2]; 	// 
	AT91_REG	 ADC_CHER; 	// ADC Channel Enable Register
	AT91_REG	 ADC_CHDR; 	// ADC Channel Disable Register
	AT91_REG	 ADC_CHSR; 	// ADC Channel Status Register
	AT91_REG	 ADC_SR; 	// ADC Status Register
	AT91_REG	 ADC_LCDR; 	// ADC Last Converted Data Register
	AT91_REG	 ADC_IER; 	// ADC Interrupt Enable Register
	AT91_REG	 ADC_IDR; 	// ADC Interrupt Disable Register
	AT91_REG	 ADC_IMR; 	// ADC Interrupt Mask Register
	AT91_REG	 ADC_CDR0; 	// ADC Channel Data Register 0
	AT91_REG	 ADC_CDR1; 	// ADC Channel Data Register 1
	AT91_REG	 ADC_CDR2; 	// ADC Channel Data Register 2
	AT91_REG	 ADC_CDR3; 	// ADC Channel Data Register 3
	AT91_REG	 ADC_CDR4; 	// ADC Channel Data Register 4
	AT91_REG	 ADC_CDR5; 	// ADC Channel Data Register 5
	AT91_REG	 ADC_CDR6; 	// ADC Channel Data Register 6
	AT91_REG	 ADC_CDR7; 	// ADC Channel Data Register 7
	AT91_REG	 Reserved1[5]; 	// 
	AT91_REG	 ADC_ACR; 	// Analog Control Register
	AT91_REG	 ADC_EMR; 	// Extended Mode Register
	AT91_REG	 Reserved2[32]; 	// 
	AT91_REG	 ADC_ADDRSIZE; 	// ADC ADDRSIZE REGISTER 
	AT91_REG	 ADC_IPNAME1; 	// ADC IPNAME1 REGISTER 
	AT91_REG	 ADC_IPNAME2; 	// ADC IPNAME2 REGISTER 
	AT91_REG	 ADC_FEATURES; 	// ADC FEATURES REGISTER 
	AT91_REG	 ADC_VER; 	// ADC VERSION REGISTER
	AT91_REG	 ADC_RPR; 	// Receive Pointer Register
	AT91_REG	 ADC_RCR; 	// Receive Counter Register
	AT91_REG	 ADC_TPR; 	// Transmit Pointer Register
	AT91_REG	 ADC_TCR; 	// Transmit Counter Register
	AT91_REG	 ADC_RNPR; 	// Receive Next Pointer Register
	AT91_REG	 ADC_RNCR; 	// Receive Next Counter Register
	AT91_REG	 ADC_TNPR; 	// Transmit Next Pointer Register
	AT91_REG	 ADC_TNCR; 	// Transmit Next Counter Register
	AT91_REG	 ADC_PTCR; 	// PDC Transfer Control Register
	AT91_REG	 ADC_PTSR; 	// PDC Transfer Status Register
} AT91S_ADC, *AT91PS_ADC;
#line 2289 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC_CR : (ADC Offset: 0x0) ADC Control Register -------- 


// -------- ADC_MR : (ADC Offset: 0x4) ADC Mode Register -------- 
#line 2313 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- 	ADC_CHER : (ADC Offset: 0x10) ADC Channel Enable Register -------- 
#line 2322 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- 	ADC_CHDR : (ADC Offset: 0x14) ADC Channel Disable Register -------- 
// -------- 	ADC_CHSR : (ADC Offset: 0x18) ADC Channel Status Register -------- 
// -------- ADC_SR : (ADC Offset: 0x1c) ADC Status Register -------- 
#line 2345 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC_LCDR : (ADC Offset: 0x20) ADC Last Converted Data Register -------- 

// -------- ADC_IER : (ADC Offset: 0x24) ADC Interrupt Enable Register -------- 
// -------- ADC_IDR : (ADC Offset: 0x28) ADC Interrupt Disable Register -------- 
// -------- ADC_IMR : (ADC Offset: 0x2c) ADC Interrupt Mask Register -------- 
// -------- ADC_CDR0 : (ADC Offset: 0x30) ADC Channel Data Register 0 -------- 

// -------- ADC_CDR1 : (ADC Offset: 0x34) ADC Channel Data Register 1 -------- 
// -------- ADC_CDR2 : (ADC Offset: 0x38) ADC Channel Data Register 2 -------- 
// -------- ADC_CDR3 : (ADC Offset: 0x3c) ADC Channel Data Register 3 -------- 
// -------- ADC_CDR4 : (ADC Offset: 0x40) ADC Channel Data Register 4 -------- 
// -------- ADC_CDR5 : (ADC Offset: 0x44) ADC Channel Data Register 5 -------- 
// -------- ADC_CDR6 : (ADC Offset: 0x48) ADC Channel Data Register 6 -------- 
// -------- ADC_CDR7 : (ADC Offset: 0x4c) ADC Channel Data Register 7 -------- 
// -------- ADC_ACR : (ADC Offset: 0x64) ADC Analog Controler Register -------- 
#line 2368 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC_EMR : (ADC Offset: 0x68) ADC Extended Mode Register -------- 


// -------- ADC_VER : (ADC Offset: 0xfc) ADC VER -------- 


// *****************************************************************************
//   SOFTWARE API DEFINITION FOR Analog-to Digital Converter
// *****************************************************************************


typedef struct _AT91S_ADC12B {
  AT91_REG ADC12B_CR;     // Control Register
  AT91_REG ADC12B_MR;     // Mode Register
  AT91_REG Reserved1[2]; 
  AT91_REG ADC12B_CHER;   // Channel Enable Register
  AT91_REG ADC12B_CHDR;   // Channel Disable Register
  AT91_REG ADC12B_CHSR;   // Channel Status Register
  AT91_REG ADC12B_SR;     // Status Register
  AT91_REG ADC12B_LCDR;   // Last Converted Data Register
  AT91_REG ADC12B_IER;    // Interrupt Enable Register
  AT91_REG ADC12B_IDR;    // Interrupt Disable Register
  AT91_REG ADC12B_IMR;    // Interrupt Mask Register
  AT91_REG ADC12B_CDR[8]; // Channel Data Register
  AT91_REG Reserved2[5]; 
  AT91_REG ADC12B_ACR;    // Analog Control Register
  AT91_REG ADC12B_EMR;    // Extended Mode Register
} AT91S_ADC12B, *AT91PS_ADC12B;
#line 2411 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_CR : (ADC12B Offset: 0x00) Control Register --------
#line 2418 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_MR : (ADC12B Offset: 0x04) Mode Register --------
#line 2438 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_CHER : (ADC12B Offset: 0x10) Channel Enable Register --------
#line 2463 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_CHDR : (ADC12B Offset: 0x14) Channel Disable Register --------
#line 2488 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_CHSR : (ADC12B Offset: 0x18) Channel Status Register --------
#line 2513 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_SR : (ADC12B Offset: 0x1C) Status Register --------
#line 2574 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_LCDR : (ADC12B Offset: 0x20) Last Converted Data Register --------

// -------- ADC12B_IER : (ADC12B Offset: 0x24) Interrupt Enable Register --------
#line 2637 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_IDR : (ADC12B Offset: 0x28) Interrupt Disable Register --------
#line 2698 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_IMR : (ADC12B Offset: 0x2C) Interrupt Mask Register --------
#line 2759 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_CDR[8] : (ADC12B Offset: 0x30) Channel Data Register --------

// -------- ADC12B_ACR : (ADC12B Offset: 0x64) Analog Control Register --------
#line 2772 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- ADC12B_EMR : (ADC12B Offset: 0x68) Extended Mode Register --------





// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Timer Counter Channel Interface
// *****************************************************************************

typedef struct _AT91S_TC {
	AT91_REG	 TC_CCR; 	// Channel Control Register
	AT91_REG	 TC_CMR; 	// Channel Mode Register (Capture Mode / Waveform Mode)
	AT91_REG	 Reserved0[2]; 	// 
	AT91_REG	 TC_CV; 	// Counter Value
	AT91_REG	 TC_RA; 	// Register A
	AT91_REG	 TC_RB; 	// Register B
	AT91_REG	 TC_RC; 	// Register C
	AT91_REG	 TC_SR; 	// Status Register
	AT91_REG	 TC_IER; 	// Interrupt Enable Register
	AT91_REG	 TC_IDR; 	// Interrupt Disable Register
	AT91_REG	 TC_IMR; 	// Interrupt Mask Register
} AT91S_TC, *AT91PS_TC;
#line 2808 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- TC_CCR : (TC Offset: 0x0) TC Channel Control Register -------- 



// -------- TC_CMR : (TC Offset: 0x4) TC Channel Mode Register: Capture Mode / Waveform Mode -------- 
#line 2906 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- TC_SR : (TC Offset: 0x20) TC Channel Status Register -------- 
#line 2918 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- TC_IER : (TC Offset: 0x24) TC Channel Interrupt Enable Register -------- 
// -------- TC_IDR : (TC Offset: 0x28) TC Channel Interrupt Disable Register -------- 
// -------- TC_IMR : (TC Offset: 0x2c) TC Channel Interrupt Mask Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Timer Counter Interface
// *****************************************************************************

typedef struct _AT91S_TCB {
	AT91S_TC	 TCB_TC0; 	// TC Channel 0
	AT91_REG	 Reserved0[4]; 	// 
	AT91S_TC	 TCB_TC1; 	// TC Channel 1
	AT91_REG	 Reserved1[4]; 	// 
	AT91S_TC	 TCB_TC2; 	// TC Channel 2
	AT91_REG	 Reserved2[4]; 	// 
	AT91_REG	 TCB_BCR; 	// TC Block Control Register
	AT91_REG	 TCB_BMR; 	// TC Block Mode Register
	AT91_REG	 Reserved3[9]; 	// 
	AT91_REG	 TCB_ADDRSIZE; 	// TC ADDRSIZE REGISTER 
	AT91_REG	 TCB_IPNAME1; 	// TC IPNAME1 REGISTER 
	AT91_REG	 TCB_IPNAME2; 	// TC IPNAME2 REGISTER 
	AT91_REG	 TCB_FEATURES; 	// TC FEATURES REGISTER 
	AT91_REG	 TCB_VER; 	//  Version Register
} AT91S_TCB, *AT91PS_TCB;
#line 2952 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- TCB_BCR : (TCB Offset: 0xc0) TC Block Control Register -------- 

// -------- TCB_BMR : (TCB Offset: 0xc4) TC Block Mode Register -------- 
#line 2970 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Embedded Flash Controller 2.0
// *****************************************************************************

typedef struct _AT91S_EFC {
	AT91_REG	 EFC_FMR; 	// EFC Flash Mode Register
	AT91_REG	 EFC_FCR; 	// EFC Flash Command Register
	AT91_REG	 EFC_FSR; 	// EFC Flash Status Register
	AT91_REG	 EFC_FRR; 	// EFC Flash Result Register
	AT91_REG	 Reserved0[1]; 	// 
	AT91_REG	 EFC_FVR; 	// EFC Flash Version Register
} AT91S_EFC, *AT91PS_EFC;
#line 2991 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- EFC_FMR : (EFC Offset: 0x0) EFC Flash Mode Register -------- 
#line 2998 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- EFC_FCR : (EFC Offset: 0x4) EFC Flash Command Register -------- 
#line 3018 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- EFC_FSR : (EFC Offset: 0x8) EFC Flash Status Register -------- 



// -------- EFC_FRR : (EFC Offset: 0xc) EFC Flash Result Register -------- 


// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Multimedia Card Interface
// *****************************************************************************

typedef struct _AT91S_MCI {
	AT91_REG	 MCI_CR; 	// MCI Control Register
	AT91_REG	 MCI_MR; 	// MCI Mode Register
	AT91_REG	 MCI_DTOR; 	// MCI Data Timeout Register
	AT91_REG	 MCI_SDCR; 	// MCI SD/SDIO Card Register
	AT91_REG	 MCI_ARGR; 	// MCI Argument Register
	AT91_REG	 MCI_CMDR; 	// MCI Command Register
	AT91_REG	 MCI_BLKR; 	// MCI Block Register
	AT91_REG	 MCI_CSTOR; 	// MCI Completion Signal Timeout Register
	AT91_REG	 MCI_RSPR[4]; 	// MCI Response Register
	AT91_REG	 MCI_RDR; 	// MCI Receive Data Register
	AT91_REG	 MCI_TDR; 	// MCI Transmit Data Register
	AT91_REG	 Reserved0[2]; 	// 
	AT91_REG	 MCI_SR; 	// MCI Status Register
	AT91_REG	 MCI_IER; 	// MCI Interrupt Enable Register
	AT91_REG	 MCI_IDR; 	// MCI Interrupt Disable Register
	AT91_REG	 MCI_IMR; 	// MCI Interrupt Mask Register
	AT91_REG	 MCI_DMA; 	// MCI DMA Configuration Register
	AT91_REG	 MCI_CFG; 	// MCI Configuration Register
	AT91_REG	 Reserved1[35]; 	// 
	AT91_REG	 MCI_WPCR; 	// MCI Write Protection Control Register
	AT91_REG	 MCI_WPSR; 	// MCI Write Protection Status Register
	AT91_REG	 MCI_ADDRSIZE; 	// MCI ADDRSIZE REGISTER 
	AT91_REG	 MCI_IPNAME1; 	// MCI IPNAME1 REGISTER 
	AT91_REG	 MCI_IPNAME2; 	// MCI IPNAME2 REGISTER 
	AT91_REG	 MCI_FEATURES; 	// MCI FEATURES REGISTER 
	AT91_REG	 MCI_VER; 	// MCI VERSION REGISTER 
	AT91_REG	 MCI_RPR; 	// Receive Pointer Register
	AT91_REG	 MCI_RCR; 	// Receive Counter Register
	AT91_REG	 MCI_TPR; 	// Transmit Pointer Register
	AT91_REG	 MCI_TCR; 	// Transmit Counter Register
	AT91_REG	 MCI_RNPR; 	// Receive Next Pointer Register
	AT91_REG	 MCI_RNCR; 	// Receive Next Counter Register
	AT91_REG	 MCI_TNPR; 	// Transmit Next Pointer Register
	AT91_REG	 MCI_TNCR; 	// Transmit Next Counter Register
	AT91_REG	 MCI_PTCR; 	// PDC Transfer Control Register
	AT91_REG	 MCI_PTSR; 	// PDC Transfer Status Register
	AT91_REG	 Reserved2[54]; 	// 
	AT91_REG	 MCI_FIFO; 	// MCI FIFO Aperture Register
} AT91S_MCI, *AT91PS_MCI;
#line 3097 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_CR : (MCI Offset: 0x0) MCI Control Register -------- 
#line 3119 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_MR : (MCI Offset: 0x4) MCI Mode Register -------- 
#line 3136 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_DTOR : (MCI Offset: 0x8) MCI Data Timeout Register -------- 
#line 3147 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_SDCR : (MCI Offset: 0xc) MCI SD Card Register -------- 
#line 3157 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_CMDR : (MCI Offset: 0x14) MCI Command Register -------- 
#line 3197 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_BLKR : (MCI Offset: 0x18) MCI Block Register -------- 

// -------- MCI_CSTOR : (MCI Offset: 0x1c) MCI Completion Signal Timeout Register -------- 
#line 3210 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_SR : (MCI Offset: 0x40) MCI Status Register -------- 
#line 3241 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_IER : (MCI Offset: 0x44) MCI Interrupt Enable Register -------- 
// -------- MCI_IDR : (MCI Offset: 0x48) MCI Interrupt Disable Register -------- 
// -------- MCI_IMR : (MCI Offset: 0x4c) MCI Interrupt Mask Register -------- 
// -------- MCI_DMA : (MCI Offset: 0x50) MCI DMA Configuration Register -------- 
#line 3255 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_CFG : (MCI Offset: 0x54) MCI Configuration Register -------- 
#line 3268 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_WPCR : (MCI Offset: 0xe4) Write Protection Control Register -------- 




// -------- MCI_WPSR : (MCI Offset: 0xe8) Write Protection Status Register -------- 
#line 3288 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- MCI_VER : (MCI Offset: 0xfc)  VERSION  Register -------- 


// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Two-wire Interface
// *****************************************************************************

typedef struct _AT91S_TWI {
	AT91_REG	 TWI_CR; 	// Control Register
	AT91_REG	 TWI_MMR; 	// Master Mode Register
	AT91_REG	 TWI_SMR; 	// Slave Mode Register
	AT91_REG	 TWI_IADR; 	// Internal Address Register
	AT91_REG	 TWI_CWGR; 	// Clock Waveform Generator Register
	AT91_REG	 Reserved0[3]; 	// 
	AT91_REG	 TWI_SR; 	// Status Register
	AT91_REG	 TWI_IER; 	// Interrupt Enable Register
	AT91_REG	 TWI_IDR; 	// Interrupt Disable Register
	AT91_REG	 TWI_IMR; 	// Interrupt Mask Register
	AT91_REG	 TWI_RHR; 	// Receive Holding Register
	AT91_REG	 TWI_THR; 	// Transmit Holding Register
	AT91_REG	 Reserved1[45]; 	// 
	AT91_REG	 TWI_ADDRSIZE; 	// TWI ADDRSIZE REGISTER 
	AT91_REG	 TWI_IPNAME1; 	// TWI IPNAME1 REGISTER 
	AT91_REG	 TWI_IPNAME2; 	// TWI IPNAME2 REGISTER 
	AT91_REG	 TWI_FEATURES; 	// TWI FEATURES REGISTER 
	AT91_REG	 TWI_VER; 	// Version Register
	AT91_REG	 TWI_RPR; 	// Receive Pointer Register
	AT91_REG	 TWI_RCR; 	// Receive Counter Register
	AT91_REG	 TWI_TPR; 	// Transmit Pointer Register
	AT91_REG	 TWI_TCR; 	// Transmit Counter Register
	AT91_REG	 TWI_RNPR; 	// Receive Next Pointer Register
	AT91_REG	 TWI_RNCR; 	// Receive Next Counter Register
	AT91_REG	 TWI_TNPR; 	// Transmit Next Pointer Register
	AT91_REG	 TWI_TNCR; 	// Transmit Next Counter Register
	AT91_REG	 TWI_PTCR; 	// PDC Transfer Control Register
	AT91_REG	 TWI_PTSR; 	// PDC Transfer Status Register
} AT91S_TWI, *AT91PS_TWI;
#line 3344 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- TWI_CR : (TWI Offset: 0x0) TWI Control Register -------- 
#line 3352 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- TWI_MMR : (TWI Offset: 0x4) TWI Master Mode Register -------- 
#line 3360 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- TWI_SMR : (TWI Offset: 0x8) TWI Slave Mode Register -------- 

// -------- TWI_CWGR : (TWI Offset: 0x10) TWI Clock Waveform Generator Register -------- 



// -------- TWI_SR : (TWI Offset: 0x20) TWI Status Register -------- 
#line 3385 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- TWI_IER : (TWI Offset: 0x24) TWI Interrupt Enable Register -------- 
// -------- TWI_IDR : (TWI Offset: 0x28) TWI Interrupt Disable Register -------- 
// -------- TWI_IMR : (TWI Offset: 0x2c) TWI Interrupt Mask Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Usart
// *****************************************************************************



typedef struct _AT91S_USART {
	AT91_REG	 US_CR; 	// Control Register
	AT91_REG	 US_MR; 	// Mode Register
	AT91_REG	 US_IER; 	// Interrupt Enable Register
	AT91_REG	 US_IDR; 	// Interrupt Disable Register
	AT91_REG	 US_IMR; 	// Interrupt Mask Register
	AT91_REG	 US_CSR; 	// Channel Status Register
	AT91_REG	 US_RHR; 	// Receiver Holding Register
	AT91_REG	 US_THR; 	// Transmitter Holding Register
	AT91_REG	 US_BRGR; 	// Baud Rate Generator Register
	AT91_REG	 US_RTOR; 	// Receiver Time-out Register
	AT91_REG	 US_TTGR; 	// Transmitter Time-guard Register
	AT91_REG	 Reserved0[5]; 	// 
	AT91_REG	 US_FIDI; 	// FI_DI_Ratio Register
	AT91_REG	 US_NER; 	// Nb Errors Register
	AT91_REG	 Reserved1[1]; 	// 
	AT91_REG	 US_IF; 	// IRDA_FILTER Register
	AT91_REG	 US_MAN; 	// Manchester Encoder Decoder Register
	AT91_REG	 Reserved2[38]; 	// 
	AT91_REG	 US_ADDRSIZE; 	// US ADDRSIZE REGISTER 
	AT91_REG	 US_IPNAME1; 	// US IPNAME1 REGISTER 
	AT91_REG	 US_IPNAME2; 	// US IPNAME2 REGISTER 
	AT91_REG	 US_FEATURES; 	// US FEATURES REGISTER 
	AT91_REG	 US_VER; 	// VERSION Register
	AT91_REG	 US_RPR; 	// Receive Pointer Register
	AT91_REG	 US_RCR; 	// Receive Counter Register
	AT91_REG	 US_TPR; 	// Transmit Pointer Register
	AT91_REG	 US_TCR; 	// Transmit Counter Register
	AT91_REG	 US_RNPR; 	// Receive Next Pointer Register
	AT91_REG	 US_RNCR; 	// Receive Next Counter Register
	AT91_REG	 US_TNPR; 	// Transmit Next Pointer Register
	AT91_REG	 US_TNCR; 	// Transmit Next Counter Register
	AT91_REG	 US_PTCR; 	// PDC Transfer Control Register
	AT91_REG	 US_PTSR; 	// PDC Transfer Status Register
} AT91S_USART, *AT91PS_USART;
#line 3453 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- US_CR : (USART Offset: 0x0)  Control Register -------- 
#line 3472 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- US_MR : (USART Offset: 0x4)  Mode Register -------- 
#line 3522 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- US_IER : (USART Offset: 0x8)  Interrupt Enable Register -------- 
#line 3542 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- US_IDR : (USART Offset: 0xc)  Interrupt Disable Register -------- 
// -------- US_IMR : (USART Offset: 0x10)  Interrupt Mask Register -------- 
// -------- US_CSR : (USART Offset: 0x14)  Channel Status Register -------- 





// -------- US_MAN : (USART Offset: 0x50) Manchester Encoder Decoder Register -------- 
#line 3566 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Synchronous Serial Controller Interface
// *****************************************************************************

typedef struct _AT91S_SSC {
	AT91_REG	 SSC_CR; 	// Control Register
	AT91_REG	 SSC_CMR; 	// Clock Mode Register
	AT91_REG	 Reserved0[2]; 	// 
	AT91_REG	 SSC_RCMR; 	// Receive Clock ModeRegister
	AT91_REG	 SSC_RFMR; 	// Receive Frame Mode Register
	AT91_REG	 SSC_TCMR; 	// Transmit Clock Mode Register
	AT91_REG	 SSC_TFMR; 	// Transmit Frame Mode Register
	AT91_REG	 SSC_RHR; 	// Receive Holding Register
	AT91_REG	 SSC_THR; 	// Transmit Holding Register
	AT91_REG	 Reserved1[2]; 	// 
	AT91_REG	 SSC_RSHR; 	// Receive Sync Holding Register
	AT91_REG	 SSC_TSHR; 	// Transmit Sync Holding Register
	AT91_REG	 SSC_RC0R; 	// Receive Compare 0 Register
	AT91_REG	 SSC_RC1R; 	// Receive Compare 1 Register
	AT91_REG	 SSC_SR; 	// Status Register
	AT91_REG	 SSC_IER; 	// Interrupt Enable Register
	AT91_REG	 SSC_IDR; 	// Interrupt Disable Register
	AT91_REG	 SSC_IMR; 	// Interrupt Mask Register
} AT91S_SSC, *AT91PS_SSC;
#line 3610 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SSC_CR : (SSC Offset: 0x0) SSC Control Register -------- 





// -------- SSC_RCMR : (SSC Offset: 0x10) SSC Receive Clock Mode Register -------- 
#line 3644 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SSC_RFMR : (SSC Offset: 0x14) SSC Receive Frame Mode Register -------- 
#line 3658 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SSC_TCMR : (SSC Offset: 0x18) SSC Transmit Clock Mode Register -------- 
// -------- SSC_TFMR : (SSC Offset: 0x1c) SSC Transmit Frame Mode Register -------- 


// -------- SSC_SR : (SSC Offset: 0x40) SSC Status Register -------- 
#line 3677 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SSC_IER : (SSC Offset: 0x44) SSC Interrupt Enable Register -------- 
// -------- SSC_IDR : (SSC Offset: 0x48) SSC Interrupt Disable Register -------- 
// -------- SSC_IMR : (SSC Offset: 0x4c) SSC Interrupt Mask Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR PWMC Channel Interface
// *****************************************************************************

typedef struct _AT91S_PWMC_CH {
	AT91_REG	 PWMC_CMR; 	// Channel Mode Register
	AT91_REG	 PWMC_CDTYR; 	// Channel Duty Cycle Register
	AT91_REG	 PWMC_CDTYUPDR; 	// Channel Duty Cycle Update Register
	AT91_REG	 PWMC_CPRDR; 	// Channel Period Register
	AT91_REG	 PWMC_CPRDUPDR; 	// Channel Period Update Register
	AT91_REG	 PWMC_CCNTR; 	// Channel Counter Register
	AT91_REG	 PWMC_DTR; 	// Channel Dead Time Value Register
	AT91_REG	 PWMC_DTUPDR; 	// Channel Dead Time Update Value Register
} AT91S_PWMC_CH, *AT91PS_PWMC_CH;
#line 3706 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_CMR : (PWMC_CH Offset: 0x0) PWMC Channel Mode Register -------- 
#line 3727 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_CDTYR : (PWMC_CH Offset: 0x4) PWMC Channel Duty Cycle Register -------- 

// -------- PWMC_CDTYUPDR : (PWMC_CH Offset: 0x8) PWMC Channel Duty Cycle Update Register -------- 

// -------- PWMC_CPRDR : (PWMC_CH Offset: 0xc) PWMC Channel Period Register -------- 

// -------- PWMC_CPRDUPDR : (PWMC_CH Offset: 0x10) PWMC Channel Period Update Register -------- 

// -------- PWMC_CCNTR : (PWMC_CH Offset: 0x14) PWMC Channel Counter Register -------- 

// -------- PWMC_DTR : (PWMC_CH Offset: 0x18) Channel Dead Time Value Register -------- 


// -------- PWMC_DTUPDR : (PWMC_CH Offset: 0x1c) Channel Dead Time Value Register -------- 



// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Pulse Width Modulation Controller Interface
// *****************************************************************************

typedef struct _AT91S_PWMC {
	AT91_REG	 PWMC_CLK; 	// PWMC Mode Register
	AT91_REG	 PWMC_ENA; 	// PWMC Enable Register
	AT91_REG	 PWMC_DIS; 	// PWMC Disable Register
	AT91_REG	 PWMC_SR; 	// PWMC Status Register
	AT91_REG	 PWMC_IER1; 	// PWMC Interrupt Enable Register 1
	AT91_REG	 PWMC_IDR1; 	// PWMC Interrupt Disable Register 1
	AT91_REG	 PWMC_IMR1; 	// PWMC Interrupt Mask Register 1
	AT91_REG	 PWMC_ISR1; 	// PWMC Interrupt Status Register 1
	AT91_REG	 PWMC_SCM;  	// PWM Synchronized Channels Register
	AT91_REG	 Reserved0[1]; 	// 
	AT91_REG	 PWMC_UPCR; 	// PWM Update Control Register
	AT91_REG	 PWMC_SCUP; 	// PWM Update Period Register
	AT91_REG	 PWMC_SCUPUPD; 	// PWM Update Period Update Register
	AT91_REG	 PWMC_IER2; 	// PWMC Interrupt Enable Register 2
	AT91_REG	 PWMC_IDR2; 	// PWMC Interrupt Disable Register 2
	AT91_REG	 PWMC_IMR2; 	// PWMC Interrupt Mask Register 2
	AT91_REG	 PWMC_ISR2; 	// PWMC Interrupt Status Register 2
	AT91_REG	 PWMC_OOV; 	// PWM Output Override Value Register
	AT91_REG	 PWMC_OS; 	// PWM Output Selection Register
	AT91_REG	 PWMC_OSS; 	// PWM Output Selection Set Register
	AT91_REG	 PWMC_OSC; 	// PWM Output Selection Clear Register
	AT91_REG	 PWMC_OSSUPD; 	// PWM Output Selection Set Update Register
	AT91_REG	 PWMC_OSCUPD; 	// PWM Output Selection Clear Update Register
	AT91_REG	 PWMC_FMR; 	// PWM Fault Mode Register
	AT91_REG	 PWMC_FSR; 	// PWM Fault Mode Status Register
	AT91_REG	 PWMC_FCR; 	// PWM Fault Mode Clear Register
	AT91_REG	 PWMC_FPV; 	// PWM Fault Protection Value Register
	AT91_REG	 PWMC_FPER1; 	// PWM Fault Protection Enable Register 1
	AT91_REG	 PWMC_FPER2; 	// PWM Fault Protection Enable Register 2
	AT91_REG	 PWMC_FPER3; 	// PWM Fault Protection Enable Register 3
	AT91_REG	 PWMC_FPER4; 	// PWM Fault Protection Enable Register 4
	AT91_REG	 PWMC_EL0MR; 	// PWM Event Line 0 Mode Register
	AT91_REG	 PWMC_EL1MR; 	// PWM Event Line 1 Mode Register
	AT91_REG	 PWMC_EL2MR; 	// PWM Event Line 2 Mode Register
	AT91_REG	 PWMC_EL3MR; 	// PWM Event Line 3 Mode Register
	AT91_REG	 PWMC_EL4MR; 	// PWM Event Line 4 Mode Register
	AT91_REG	 PWMC_EL5MR; 	// PWM Event Line 5 Mode Register
	AT91_REG	 PWMC_EL6MR; 	// PWM Event Line 6 Mode Register
	AT91_REG	 PWMC_EL7MR; 	// PWM Event Line 7 Mode Register
	AT91_REG	 Reserved1[18]; 	// 
	AT91_REG	 PWMC_WPCR; 	// PWM Write Protection Enable Register
	AT91_REG	 PWMC_WPSR; 	// PWM Write Protection Status Register
	AT91_REG	 PWMC_ADDRSIZE; 	// PWMC ADDRSIZE REGISTER 
	AT91_REG	 PWMC_IPNAME1; 	// PWMC IPNAME1 REGISTER 
	AT91_REG	 PWMC_IPNAME2; 	// PWMC IPNAME2 REGISTER 
	AT91_REG	 PWMC_FEATURES; 	// PWMC FEATURES REGISTER 
	AT91_REG	 PWMC_VER; 	// PWMC Version Register
	AT91_REG	 PWMC_RPR; 	// Receive Pointer Register
	AT91_REG	 PWMC_RCR; 	// Receive Counter Register
	AT91_REG	 PWMC_TPR; 	// Transmit Pointer Register
	AT91_REG	 PWMC_TCR; 	// Transmit Counter Register
	AT91_REG	 PWMC_RNPR; 	// Receive Next Pointer Register
	AT91_REG	 PWMC_RNCR; 	// Receive Next Counter Register
	AT91_REG	 PWMC_TNPR; 	// Transmit Next Pointer Register
	AT91_REG	 PWMC_TNCR; 	// Transmit Next Counter Register
	AT91_REG	 PWMC_PTCR; 	// PDC Transfer Control Register
	AT91_REG	 PWMC_PTSR; 	// PDC Transfer Status Register
	AT91_REG	 Reserved2[2]; 	// 
	AT91_REG	 PWMC_CMP0V; 	// PWM Comparison Value 0 Register
	AT91_REG	 PWMC_CMP0VUPD; 	// PWM Comparison Value 0 Update Register
	AT91_REG	 PWMC_CMP0M; 	// PWM Comparison Mode 0 Register
	AT91_REG	 PWMC_CMP0MUPD; 	// PWM Comparison Mode 0 Update Register
	AT91_REG	 PWMC_CMP1V; 	// PWM Comparison Value 1 Register
	AT91_REG	 PWMC_CMP1VUPD; 	// PWM Comparison Value 1 Update Register
	AT91_REG	 PWMC_CMP1M; 	// PWM Comparison Mode 1 Register
	AT91_REG	 PWMC_CMP1MUPD; 	// PWM Comparison Mode 1 Update Register
	AT91_REG	 PWMC_CMP2V; 	// PWM Comparison Value 2 Register
	AT91_REG	 PWMC_CMP2VUPD; 	// PWM Comparison Value 2 Update Register
	AT91_REG	 PWMC_CMP2M; 	// PWM Comparison Mode 2 Register
	AT91_REG	 PWMC_CMP2MUPD; 	// PWM Comparison Mode 2 Update Register
	AT91_REG	 PWMC_CMP3V; 	// PWM Comparison Value 3 Register
	AT91_REG	 PWMC_CMP3VUPD; 	// PWM Comparison Value 3 Update Register
	AT91_REG	 PWMC_CMP3M; 	// PWM Comparison Mode 3 Register
	AT91_REG	 PWMC_CMP3MUPD; 	// PWM Comparison Mode 3 Update Register
	AT91_REG	 PWMC_CMP4V; 	// PWM Comparison Value 4 Register
	AT91_REG	 PWMC_CMP4VUPD; 	// PWM Comparison Value 4 Update Register
	AT91_REG	 PWMC_CMP4M; 	// PWM Comparison Mode 4 Register
	AT91_REG	 PWMC_CMP4MUPD; 	// PWM Comparison Mode 4 Update Register
	AT91_REG	 PWMC_CMP5V; 	// PWM Comparison Value 5 Register
	AT91_REG	 PWMC_CMP5VUPD; 	// PWM Comparison Value 5 Update Register
	AT91_REG	 PWMC_CMP5M; 	// PWM Comparison Mode 5 Register
	AT91_REG	 PWMC_CMP5MUPD; 	// PWM Comparison Mode 5 Update Register
	AT91_REG	 PWMC_CMP6V; 	// PWM Comparison Value 6 Register
	AT91_REG	 PWMC_CMP6VUPD; 	// PWM Comparison Value 6 Update Register
	AT91_REG	 PWMC_CMP6M; 	// PWM Comparison Mode 6 Register
	AT91_REG	 PWMC_CMP6MUPD; 	// PWM Comparison Mode 6 Update Register
	AT91_REG	 PWMC_CMP7V; 	// PWM Comparison Value 7 Register
	AT91_REG	 PWMC_CMP7VUPD; 	// PWM Comparison Value 7 Update Register
	AT91_REG	 PWMC_CMP7M; 	// PWM Comparison Mode 7 Register
	AT91_REG	 PWMC_CMP7MUPD; 	// PWM Comparison Mode 7 Update Register
	AT91_REG	 Reserved3[20]; 	// 
	AT91S_PWMC_CH	 PWMC_CH[8]; 	// PWMC Channel 0
} AT91S_PWMC, *AT91PS_PWMC;
#line 3922 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_MR : (PWMC Offset: 0x0) PWMC Mode Register -------- 
#line 3946 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_ENA : (PWMC Offset: 0x4) PWMC Enable Register -------- 
#line 3963 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_DIS : (PWMC Offset: 0x8) PWMC Disable Register -------- 
// -------- PWMC_SR : (PWMC Offset: 0xc) PWMC Status Register -------- 
// -------- PWMC_IER1 : (PWMC Offset: 0x10) PWMC Interrupt Enable Register -------- 
#line 3982 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_IDR1 : (PWMC Offset: 0x14) PWMC Interrupt Disable Register -------- 
// -------- PWMC_IMR1 : (PWMC Offset: 0x18) PWMC Interrupt Mask Register -------- 
// -------- PWMC_ISR1 : (PWMC Offset: 0x1c) PWMC Interrupt Status Register -------- 
// -------- PWMC_SYNC : (PWMC Offset: 0x20) PWMC Synchronous Channels Register -------- 
#line 4006 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_UPCR : (PWMC Offset: 0x28) PWMC Update Control Register -------- 

// -------- PWMC_SCUP : (PWMC Offset: 0x2c) PWM Update Period Register -------- 


// -------- PWMC_SCUPUPD : (PWMC Offset: 0x30) PWM Update Period Update Register -------- 

// -------- PWMC_IER2 : (PWMC Offset: 0x34) PWMC Interrupt Enable Register -------- 




// -------- PWMC_IDR2 : (PWMC Offset: 0x38) PWMC Interrupt Disable Register -------- 
// -------- PWMC_IMR2 : (PWMC Offset: 0x3c) PWMC Interrupt Mask Register -------- 
// -------- PWMC_ISR2 : (PWMC Offset: 0x40) PWMC Interrupt Status Register -------- 
#line 4037 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_OOV : (PWMC Offset: 0x44) PWM Output Override Value Register -------- 
#line 4070 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_OS : (PWMC Offset: 0x48) PWM Output Selection Register -------- 
#line 4103 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_OSS : (PWMC Offset: 0x4c) PWM Output Selection Set Register -------- 
#line 4136 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_OSC : (PWMC Offset: 0x50) PWM Output Selection Clear Register -------- 
#line 4169 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_OSSUPD : (PWMC Offset: 0x54) Output Selection Set for PWMH / PWML output of the channel x -------- 
#line 4202 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_OSCUPD : (PWMC Offset: 0x58) Output Selection Clear for PWMH / PWML output of the channel x -------- 
#line 4235 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_FMR : (PWMC Offset: 0x5c) PWM Fault Mode Register -------- 
#line 4260 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_FSR : (PWMC Offset: 0x60) Fault Input x Value -------- 
#line 4277 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_FCR : (PWMC Offset: 0x64) Fault y Clear -------- 
#line 4286 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_FPV : (PWMC Offset: 0x68) PWM Fault Protection Value -------- 
#line 4303 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_FPER1 : (PWMC Offset: 0x6c) PWM Fault Protection Enable Register 1 -------- 




// -------- PWMC_FPER2 : (PWMC Offset: 0x70) PWM Fault Protection Enable Register 2 -------- 




// -------- PWMC_FPER3 : (PWMC Offset: 0x74) PWM Fault Protection Enable Register 3 -------- 




// -------- PWMC_FPER4 : (PWMC Offset: 0x78) PWM Fault Protection Enable Register 4 -------- 




// -------- PWMC_EL0MR : (PWMC Offset: 0x7c) PWM Event Line 0 Mode Register -------- 
#line 4332 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_EL1MR : (PWMC Offset: 0x80) PWM Event Line 1 Mode Register -------- 
#line 4341 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_EL2MR : (PWMC Offset: 0x84) PWM Event line 2 Mode Register -------- 
#line 4350 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_EL3MR : (PWMC Offset: 0x88) PWM Event line 3 Mode Register -------- 
#line 4359 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_EL4MR : (PWMC Offset: 0x8c) PWM Event line 4 Mode Register -------- 
#line 4368 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_EL5MR : (PWMC Offset: 0x90) PWM Event line 5 Mode Register -------- 
#line 4377 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_EL6MR : (PWMC Offset: 0x94) PWM Event line 6 Mode Register -------- 
#line 4386 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_EL7MR : (PWMC Offset: 0x98) PWM Event line 7 Mode Register -------- 
#line 4395 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_WPCR : (PWMC Offset: 0xe4) PWM Write Protection Control Register -------- 
#line 4404 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_WPVS : (PWMC Offset: 0xe8) Write Protection Status Register -------- 
#line 4419 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_CMP0V : (PWMC Offset: 0x130) PWM Comparison Value 0 Register -------- 


// -------- PWMC_CMP0VUPD : (PWMC Offset: 0x134) PWM Comparison Value 0 Update Register -------- 


// -------- PWMC_CMP0M : (PWMC Offset: 0x138) PWM Comparison 0 Mode Register -------- 
#line 4432 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- PWMC_CMP0MUPD : (PWMC Offset: 0x13c) PWM Comparison 0 Mode Update Register -------- 




// -------- PWMC_CMP1V : (PWMC Offset: 0x140) PWM Comparison Value 1 Register -------- 
// -------- PWMC_CMP1VUPD : (PWMC Offset: 0x144) PWM Comparison Value 1 Update Register -------- 
// -------- PWMC_CMP1M : (PWMC Offset: 0x148) PWM Comparison 1 Mode Register -------- 
// -------- PWMC_CMP1MUPD : (PWMC Offset: 0x14c) PWM Comparison 1 Mode Update Register -------- 
// -------- PWMC_CMP2V : (PWMC Offset: 0x150) PWM Comparison Value 2 Register -------- 
// -------- PWMC_CMP2VUPD : (PWMC Offset: 0x154) PWM Comparison Value 2 Update Register -------- 
// -------- PWMC_CMP2M : (PWMC Offset: 0x158) PWM Comparison 2 Mode Register -------- 
// -------- PWMC_CMP2MUPD : (PWMC Offset: 0x15c) PWM Comparison 2 Mode Update Register -------- 
// -------- PWMC_CMP3V : (PWMC Offset: 0x160) PWM Comparison Value 3 Register -------- 
// -------- PWMC_CMP3VUPD : (PWMC Offset: 0x164) PWM Comparison Value 3 Update Register -------- 
// -------- PWMC_CMP3M : (PWMC Offset: 0x168) PWM Comparison 3 Mode Register -------- 
// -------- PWMC_CMP3MUPD : (PWMC Offset: 0x16c) PWM Comparison 3 Mode Update Register -------- 
// -------- PWMC_CMP4V : (PWMC Offset: 0x170) PWM Comparison Value 4 Register -------- 
// -------- PWMC_CMP4VUPD : (PWMC Offset: 0x174) PWM Comparison Value 4 Update Register -------- 
// -------- PWMC_CMP4M : (PWMC Offset: 0x178) PWM Comparison 4 Mode Register -------- 
// -------- PWMC_CMP4MUPD : (PWMC Offset: 0x17c) PWM Comparison 4 Mode Update Register -------- 
// -------- PWMC_CMP5V : (PWMC Offset: 0x180) PWM Comparison Value 5 Register -------- 
// -------- PWMC_CMP5VUPD : (PWMC Offset: 0x184) PWM Comparison Value 5 Update Register -------- 
// -------- PWMC_CMP5M : (PWMC Offset: 0x188) PWM Comparison 5 Mode Register -------- 
// -------- PWMC_CMP5MUPD : (PWMC Offset: 0x18c) PWM Comparison 5 Mode Update Register -------- 
// -------- PWMC_CMP6V : (PWMC Offset: 0x190) PWM Comparison Value 6 Register -------- 
// -------- PWMC_CMP6VUPD : (PWMC Offset: 0x194) PWM Comparison Value 6 Update Register -------- 
// -------- PWMC_CMP6M : (PWMC Offset: 0x198) PWM Comparison 6 Mode Register -------- 
// -------- PWMC_CMP6MUPD : (PWMC Offset: 0x19c) PWM Comparison 6 Mode Update Register -------- 
// -------- PWMC_CMP7V : (PWMC Offset: 0x1a0) PWM Comparison Value 7 Register -------- 
// -------- PWMC_CMP7VUPD : (PWMC Offset: 0x1a4) PWM Comparison Value 7 Update Register -------- 
// -------- PWMC_CMP7M : (PWMC Offset: 0x1a8) PWM Comparison 7 Mode Register -------- 
// -------- PWMC_CMP7MUPD : (PWMC Offset: 0x1ac) PWM Comparison 7 Mode Update Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Serial Parallel Interface
// *****************************************************************************

typedef struct _AT91S_SPI {
	AT91_REG	 SPI_CR; 	// Control Register
	AT91_REG	 SPI_MR; 	// Mode Register
	AT91_REG	 SPI_RDR; 	// Receive Data Register
	AT91_REG	 SPI_TDR; 	// Transmit Data Register
	AT91_REG	 SPI_SR; 	// Status Register
	AT91_REG	 SPI_IER; 	// Interrupt Enable Register
	AT91_REG	 SPI_IDR; 	// Interrupt Disable Register
	AT91_REG	 SPI_IMR; 	// Interrupt Mask Register
	AT91_REG	 Reserved0[4]; 	// 
	AT91_REG	 SPI_CSR[4]; 	// Chip Select Register
	AT91_REG	 Reserved1[43]; 	// 
	AT91_REG	 SPI_ADDRSIZE; 	// SPI ADDRSIZE REGISTER 
	AT91_REG	 SPI_IPNAME1; 	// SPI IPNAME1 REGISTER 
	AT91_REG	 SPI_IPNAME2; 	// SPI IPNAME2 REGISTER 
	AT91_REG	 SPI_FEATURES; 	// SPI FEATURES REGISTER 
	AT91_REG	 SPI_VER; 	// Version Register
} AT91S_SPI, *AT91PS_SPI;
#line 4505 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SPI_CR : (SPI Offset: 0x0) SPI Control Register -------- 




// -------- SPI_MR : (SPI Offset: 0x4) SPI Mode Register -------- 
#line 4521 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SPI_RDR : (SPI Offset: 0x8) Receive Data Register -------- 


// -------- SPI_TDR : (SPI Offset: 0xc) Transmit Data Register -------- 


// -------- SPI_SR : (SPI Offset: 0x10) Status Register -------- 
#line 4539 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- SPI_IER : (SPI Offset: 0x14) Interrupt Enable Register -------- 
// -------- SPI_IDR : (SPI Offset: 0x18) Interrupt Disable Register -------- 
// -------- SPI_IMR : (SPI Offset: 0x1c) Interrupt Mask Register -------- 
// -------- SPI_CSR : (SPI Offset: 0x30) Chip Select Register -------- 
#line 4560 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR UDPHS Enpoint FIFO data register
// *****************************************************************************

typedef struct _AT91S_UDPHS_EPTFIFO {
	AT91_REG	 UDPHS_READEPT0[16384]; 	// FIFO Endpoint Data Register 0
	AT91_REG	 UDPHS_READEPT1[16384]; 	// FIFO Endpoint Data Register 1
	AT91_REG	 UDPHS_READEPT2[16384]; 	// FIFO Endpoint Data Register 2
	AT91_REG	 UDPHS_READEPT3[16384]; 	// FIFO Endpoint Data Register 3
	AT91_REG	 UDPHS_READEPT4[16384]; 	// FIFO Endpoint Data Register 4
	AT91_REG	 UDPHS_READEPT5[16384]; 	// FIFO Endpoint Data Register 5
	AT91_REG	 UDPHS_READEPT6[16384]; 	// FIFO Endpoint Data Register 6
} AT91S_UDPHS_EPTFIFO, *AT91PS_UDPHS_EPTFIFO;
#line 4584 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR UDPHS Endpoint struct
// *****************************************************************************

typedef struct _AT91S_UDPHS_EPT {
	AT91_REG	 UDPHS_EPTCFG; 	// UDPHS Endpoint Config Register
	AT91_REG	 UDPHS_EPTCTLENB; 	// UDPHS Endpoint Control Enable Register
	AT91_REG	 UDPHS_EPTCTLDIS; 	// UDPHS Endpoint Control Disable Register
	AT91_REG	 UDPHS_EPTCTL; 	// UDPHS Endpoint Control Register
	AT91_REG	 Reserved0[1]; 	// 
	AT91_REG	 UDPHS_EPTSETSTA; 	// UDPHS Endpoint Set Status Register
	AT91_REG	 UDPHS_EPTCLRSTA; 	// UDPHS Endpoint Clear Status Register
	AT91_REG	 UDPHS_EPTSTA; 	// UDPHS Endpoint Status Register
} AT91S_UDPHS_EPT, *AT91PS_UDPHS_EPT;
#line 4609 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_EPTCFG : (UDPHS_EPT Offset: 0x0) UDPHS Endpoint Config Register -------- 
#line 4634 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_EPTCTLENB : (UDPHS_EPT Offset: 0x4) UDPHS Endpoint Control Enable Register -------- 
#line 4654 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_EPTCTLDIS : (UDPHS_EPT Offset: 0x8) UDPHS Endpoint Control Disable Register -------- 

// -------- UDPHS_EPTCTL : (UDPHS_EPT Offset: 0xc) UDPHS Endpoint Control Register -------- 
// -------- UDPHS_EPTSETSTA : (UDPHS_EPT Offset: 0x14) UDPHS Endpoint Set Status Register -------- 


// -------- UDPHS_EPTCLRSTA : (UDPHS_EPT Offset: 0x18) UDPHS Endpoint Clear Status Register -------- 

// -------- UDPHS_EPTSTA : (UDPHS_EPT Offset: 0x1c) UDPHS Endpoint Status Register -------- 
#line 4684 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR UDPHS DMA struct
// *****************************************************************************

typedef struct _AT91S_UDPHS_DMA {
	AT91_REG	 UDPHS_DMANXTDSC; 	// UDPHS DMA Channel Next Descriptor Address
	AT91_REG	 UDPHS_DMAADDRESS; 	// UDPHS DMA Channel Address Register
	AT91_REG	 UDPHS_DMACONTROL; 	// UDPHS DMA Channel Control Register
	AT91_REG	 UDPHS_DMASTATUS; 	// UDPHS DMA Channel Status Register
} AT91S_UDPHS_DMA, *AT91PS_UDPHS_DMA;
#line 4702 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_DMANXTDSC : (UDPHS_DMA Offset: 0x0) UDPHS DMA Next Descriptor Address Register -------- 

// -------- UDPHS_DMAADDRESS : (UDPHS_DMA Offset: 0x4) UDPHS DMA Channel Address Register -------- 

// -------- UDPHS_DMACONTROL : (UDPHS_DMA Offset: 0x8) UDPHS DMA Channel Control Register -------- 
#line 4716 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_DMASTATUS : (UDPHS_DMA Offset: 0xc) UDPHS DMA Channelx Status Register -------- 






// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR UDPHS High Speed Device Interface
// *****************************************************************************

typedef struct _AT91S_UDPHS {
	AT91_REG	 UDPHS_CTRL; 	// UDPHS Control Register
	AT91_REG	 UDPHS_FNUM; 	// UDPHS Frame Number Register
	AT91_REG	 Reserved0[2]; 	// 
	AT91_REG	 UDPHS_IEN; 	// UDPHS Interrupt Enable Register
	AT91_REG	 UDPHS_INTSTA; 	// UDPHS Interrupt Status Register
	AT91_REG	 UDPHS_CLRINT; 	// UDPHS Clear Interrupt Register
	AT91_REG	 UDPHS_EPTRST; 	// UDPHS Endpoints Reset Register
	AT91_REG	 Reserved1[44]; 	// 
	AT91_REG	 UDPHS_TSTSOFCNT; 	// UDPHS Test SOF Counter Register
	AT91_REG	 UDPHS_TSTCNTA; 	// UDPHS Test A Counter Register
	AT91_REG	 UDPHS_TSTCNTB; 	// UDPHS Test B Counter Register
	AT91_REG	 UDPHS_TSTMODREG; 	// UDPHS Test Mode Register
	AT91_REG	 UDPHS_TST; 	// UDPHS Test Register
	AT91_REG	 Reserved2[2]; 	// 
	AT91_REG	 UDPHS_RIPPADDRSIZE; 	// UDPHS PADDRSIZE Register
	AT91_REG	 UDPHS_RIPNAME1; 	// UDPHS Name1 Register
	AT91_REG	 UDPHS_RIPNAME2; 	// UDPHS Name2 Register
	AT91_REG	 UDPHS_IPFEATURES; 	// UDPHS Features Register
	AT91_REG	 UDPHS_IPVERSION; 	// UDPHS Version Register
	AT91S_UDPHS_EPT	 UDPHS_EPT[7]; 	// UDPHS Endpoint struct
	AT91_REG	 Reserved3[72]; 	// 
	AT91S_UDPHS_DMA	 UDPHS_DMA[6]; 	// UDPHS DMA channel struct (not use [0])
} AT91S_UDPHS, *AT91PS_UDPHS;
#line 4770 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_CTRL : (UDPHS Offset: 0x0) UDPHS Control Register -------- 
#line 4777 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_FNUM : (UDPHS Offset: 0x4) UDPHS Frame Number Register -------- 



// -------- UDPHS_IEN : (UDPHS Offset: 0x10) UDPHS Interrupt Enable Register -------- 
#line 4802 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_INTSTA : (UDPHS Offset: 0x14) UDPHS Interrupt Status Register -------- 

// -------- UDPHS_CLRINT : (UDPHS Offset: 0x18) UDPHS Clear Interrupt Register -------- 
// -------- UDPHS_EPTRST : (UDPHS Offset: 0x1c) UDPHS Endpoints Reset Register -------- 
#line 4813 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_TSTSOFCNT : (UDPHS Offset: 0xd0) UDPHS Test SOF Counter Register -------- 


// -------- UDPHS_TSTCNTA : (UDPHS Offset: 0xd4) UDPHS Test A Counter Register -------- 


// -------- UDPHS_TSTCNTB : (UDPHS Offset: 0xd8) UDPHS Test B Counter Register -------- 


// -------- UDPHS_TSTMODREG : (UDPHS Offset: 0xdc) UDPHS Test Mode Register -------- 

// -------- UDPHS_TST : (UDPHS Offset: 0xe0) UDPHS Test Register -------- 
#line 4834 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_RIPPADDRSIZE : (UDPHS Offset: 0xec) UDPHS PADDRSIZE Register -------- 

// -------- UDPHS_RIPNAME1 : (UDPHS Offset: 0xf0) UDPHS Name Register -------- 

// -------- UDPHS_RIPNAME2 : (UDPHS Offset: 0xf4) UDPHS Name Register -------- 

// -------- UDPHS_IPFEATURES : (UDPHS Offset: 0xf8) UDPHS Features Register -------- 
#line 4852 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- UDPHS_IPVERSION : (UDPHS Offset: 0xfc) UDPHS Version Register -------- 



// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR HDMA Channel structure
// *****************************************************************************

typedef struct _AT91S_HDMA_CH {
	AT91_REG	 HDMA_SADDR; 	// HDMA Channel Source Address Register
	AT91_REG	 HDMA_DADDR; 	// HDMA Channel Destination Address Register
	AT91_REG	 HDMA_DSCR; 	// HDMA Channel Descriptor Address Register
	AT91_REG	 HDMA_CTRLA; 	// HDMA Channel Control A Register
	AT91_REG	 HDMA_CTRLB; 	// HDMA Channel Control B Register
	AT91_REG	 HDMA_CFG; 	// HDMA Channel Configuration Register
} AT91S_HDMA_CH, *AT91PS_HDMA_CH;
#line 4877 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_SADDR : (HDMA_CH Offset: 0x0)  -------- 

// -------- HDMA_DADDR : (HDMA_CH Offset: 0x4)  -------- 

// -------- HDMA_DSCR : (HDMA_CH Offset: 0x8)  -------- 

// -------- HDMA_CTRLA : (HDMA_CH Offset: 0xc)  -------- 
#line 4900 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_CTRLB : (HDMA_CH Offset: 0x10)  -------- 
#line 4921 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_CFG : (HDMA_CH Offset: 0x14)  -------- 
#line 4963 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR HDMA controller
// *****************************************************************************

typedef struct _AT91S_HDMA {
	AT91_REG	 HDMA_GCFG; 	// HDMA Global Configuration Register
	AT91_REG	 HDMA_EN; 	// HDMA Controller Enable Register
	AT91_REG	 HDMA_SREQ; 	// HDMA Software Single Request Register
	AT91_REG	 HDMA_CREQ; 	// HDMA Software Chunk Transfer Request Register
	AT91_REG	 HDMA_LAST; 	// HDMA Software Last Transfer Flag Register
	AT91_REG	 Reserved0[1]; 	// 
	AT91_REG	 HDMA_EBCIER; 	// HDMA Error, Chained Buffer transfer completed and Buffer transfer completed Interrupt Enable register
	AT91_REG	 HDMA_EBCIDR; 	// HDMA Error, Chained Buffer transfer completed and Buffer transfer completed Interrupt Disable register
	AT91_REG	 HDMA_EBCIMR; 	// HDMA Error, Chained Buffer transfer completed and Buffer transfer completed Mask Register
	AT91_REG	 HDMA_EBCISR; 	// HDMA Error, Chained Buffer transfer completed and Buffer transfer completed Status Register
	AT91_REG	 HDMA_CHER; 	// HDMA Channel Handler Enable Register
	AT91_REG	 HDMA_CHDR; 	// HDMA Channel Handler Disable Register
	AT91_REG	 HDMA_CHSR; 	// HDMA Channel Handler Status Register
	AT91_REG	 Reserved1[2]; 	// 
	AT91S_HDMA_CH	 HDMA_CH[4]; 	// HDMA Channel structure
	AT91_REG	 Reserved2[84]; 	// 
	AT91_REG	 HDMA_ADDRSIZE; 	// HDMA ADDRSIZE REGISTER 
	AT91_REG	 HDMA_IPNAME1; 	// HDMA IPNAME1 REGISTER 
	AT91_REG	 HDMA_IPNAME2; 	// HDMA IPNAME2 REGISTER 
	AT91_REG	 HDMA_FEATURES; 	// HDMA FEATURES REGISTER 
	AT91_REG	 HDMA_VER; 	// HDMA VERSION REGISTER 
} AT91S_HDMA, *AT91PS_HDMA;
#line 5011 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_GCFG : (HDMA Offset: 0x0)  -------- 



// -------- HDMA_EN : (HDMA Offset: 0x4)  -------- 



// -------- HDMA_SREQ : (HDMA Offset: 0x8)  -------- 
#line 5044 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_CREQ : (HDMA Offset: 0xc)  -------- 
#line 5069 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_LAST : (HDMA Offset: 0x10)  -------- 
#line 5094 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_EBCIER : (HDMA Offset: 0x18) Buffer Transfer Completed/Chained Buffer Transfer Completed/Access Error Interrupt Enable Register -------- 
#line 5119 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_EBCIDR : (HDMA Offset: 0x1c)  -------- 
// -------- HDMA_EBCIMR : (HDMA Offset: 0x20)  -------- 
// -------- HDMA_EBCISR : (HDMA Offset: 0x24)  -------- 
// -------- HDMA_CHER : (HDMA Offset: 0x28)  -------- 
#line 5195 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_CHDR : (HDMA Offset: 0x2c)  -------- 
#line 5244 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_CHSR : (HDMA Offset: 0x30)  -------- 
#line 5293 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// -------- HDMA_VER : (HDMA Offset: 0x1fc)  -------- 

// *****************************************************************************
//               REGISTER ADDRESS DEFINITION FOR AT91SAM3U4
// *****************************************************************************
// ========== Register definition for SYS peripheral ========== 

// ========== Register definition for HSMC4_CS0 peripheral ========== 





// ========== Register definition for HSMC4_CS1 peripheral ========== 





// ========== Register definition for HSMC4_CS2 peripheral ========== 





// ========== Register definition for HSMC4_CS3 peripheral ========== 





// ========== Register definition for HSMC4_NFC peripheral ========== 





// ========== Register definition for HSMC4 peripheral ========== 
#line 5370 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for MATRIX peripheral ========== 
#line 5410 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for NVIC peripheral ========== 
#line 5464 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for MPU peripheral ========== 
#line 5476 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for CM3 peripheral ========== 
#line 5485 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PDC_DBGU peripheral ========== 
#line 5496 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for DBGU peripheral ========== 
#line 5514 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PIOA peripheral ========== 
#line 5566 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PIOB peripheral ========== 
#line 5618 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PIOC peripheral ========== 
#line 5670 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PMC peripheral ========== 
#line 5695 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for CKGR peripheral ========== 




// ========== Register definition for RSTC peripheral ========== 




// ========== Register definition for SUPC peripheral ========== 
#line 5712 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for RTTC peripheral ========== 




// ========== Register definition for WDTC peripheral ========== 



// ========== Register definition for RTC peripheral ========== 
#line 5734 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for ADC0 peripheral ========== 
#line 5760 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for ADC12B peripheral ==========
#line 5774 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for TC0 peripheral ========== 
#line 5785 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for TC1 peripheral ========== 
#line 5796 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for TC2 peripheral ========== 
#line 5807 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for TCB0 peripheral ========== 
#line 5815 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for TCB1 peripheral ========== 
#line 5823 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for TCB2 peripheral ========== 
#line 5831 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for EFC0 peripheral ========== 





// ========== Register definition for EFC1 peripheral ========== 





// ========== Register definition for MCI0 peripheral ========== 
#line 5869 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PDC_TWI0 peripheral ========== 
#line 5880 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PDC_TWI1 peripheral ========== 
#line 5891 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for TWI0 peripheral ========== 
#line 5908 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for TWI1 peripheral ========== 
#line 5925 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PDC_US0 peripheral ========== 
#line 5936 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for US0 peripheral ========== 
#line 5957 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PDC_US1 peripheral ========== 
#line 5968 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for US1 peripheral ========== 
#line 5989 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PDC_US2 peripheral ========== 
#line 6000 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for US2 peripheral ========== 
#line 6021 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PDC_US3 peripheral ========== 
#line 6032 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for US3 peripheral ========== 
#line 6053 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PDC_SSC0 peripheral ========== 
#line 6064 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for SSC0 peripheral ========== 
#line 6081 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PDC_PWMC peripheral ========== 
#line 6092 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PWMC_CH0 peripheral ========== 
#line 6101 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PWMC_CH1 peripheral ========== 
#line 6110 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PWMC_CH2 peripheral ========== 
#line 6119 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PWMC_CH3 peripheral ========== 
#line 6128 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for PWMC peripheral ========== 
#line 6206 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for SPI0 peripheral ========== 
#line 6221 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for UDPHS_EPTFIFO peripheral ========== 
#line 6229 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for UDPHS_EPT_0 peripheral ========== 
#line 6237 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for UDPHS_EPT_1 peripheral ========== 
#line 6245 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for UDPHS_EPT_2 peripheral ========== 
#line 6253 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for UDPHS_EPT_3 peripheral ========== 
#line 6261 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for UDPHS_EPT_4 peripheral ========== 
#line 6269 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for UDPHS_EPT_5 peripheral ========== 
#line 6277 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for UDPHS_EPT_6 peripheral ========== 
#line 6285 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for UDPHS_DMA_1 peripheral ========== 




// ========== Register definition for UDPHS_DMA_2 peripheral ========== 




// ========== Register definition for UDPHS_DMA_3 peripheral ========== 




// ========== Register definition for UDPHS_DMA_4 peripheral ========== 




// ========== Register definition for UDPHS_DMA_5 peripheral ========== 




// ========== Register definition for UDPHS_DMA_6 peripheral ========== 




// ========== Register definition for UDPHS peripheral ========== 
#line 6332 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for HDMA_CH_0 peripheral ========== 
#line 6339 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for HDMA_CH_1 peripheral ========== 
#line 6346 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for HDMA_CH_2 peripheral ========== 
#line 6353 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for HDMA_CH_3 peripheral ========== 
#line 6360 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// ========== Register definition for HDMA peripheral ========== 
#line 6378 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//               PIO DEFINITIONS FOR AT91SAM3U4
// *****************************************************************************
#line 6652 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//               PERIPHERAL ID DEFINITIONS FOR AT91SAM3U4
// *****************************************************************************
#line 6687 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//               BASE ADDRESS DEFINITIONS FOR AT91SAM3U4
// How do these casts work?
// AT91_CAST does nothing for C code, so essentially the type cast is just the generic peripheral struct AT91PS_x
// So AT91C_BASE_X should just be an address to the starting peripheral register address 
// *****************************************************************************
#line 6769 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"

// *****************************************************************************
//               MEMORY MAPPING DEFINITIONS FOR AT91SAM3U4
// *****************************************************************************
// ITCM


// DTCM


// IRAM


// IRAM_MIN


// IROM


// IFLASH0
#line 6795 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// IFLASH1
#line 6802 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\AT91SAM3U4.h"
// EBI_CS0


// EBI_CS1


// EBI_SDRAM


// EBI_SDRAM_16BIT


// EBI_SDRAM_32BIT


// EBI_CS2


// EBI_CS3


// EBI_SM


// EBI_CS4


// EBI_CF0


// EBI_CS5


// EBI_CF1



#line 36 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\configuration.h"
#line 1 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\drivers\\exceptions.h"
/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/*
** This file contains the default exception handlers
** and exception table.
*/

//------------------------------------------------------------------------------
//         Types
//------------------------------------------------------------------------------

/// Function prototype for exception table items - interrupt handler.
//typedef void( *IrqHandler )( void );
//typedef void( *IntFunc )( void );

/// Weak attribute


//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Exception Handlers
//------------------------------------------------------------------------------

extern __weak void NMI_Handler( void );
extern __weak void HardFault_Handler( void );
extern __weak void MemManage_Handler( void );
extern __weak void BusFault_Handler( void );
extern __weak void UsageFault_Handler( void );
extern __weak void SVC_Handler( void );
extern __weak void DebugMon_Handler( void );
extern __weak void PendSV_Handler( void );
extern __weak void SysTick_Handler( void );
void IrqHandlerNotUsed(void);


// System Controller
extern void SYS_IrqHandler(void);
// SUPPLY CONTROLLER
extern __weak void SUPC_IrqHandler(void);
// RESET CONTROLLER
extern __weak void RSTC_IrqHandler(void);
// REAL TIME CLOCK
extern __weak void RTC_IrqHandler(void);
// REAL TIME TIMER
extern __weak void RTT_IrqHandler(void);
// WATCHDOG TIMER
extern __weak void WDT_IrqHandler(void);
// PMC
extern __weak void PMC_IrqHandler(void);
// EFC0
extern __weak void EFC0_IrqHandler(void);
// EFC1
extern __weak void EFC1_IrqHandler(void);
// DBGU
extern __weak void DBGU_IrqHandler(void);
// HSMC4
extern __weak void HSMC4_IrqHandler(void);
// Parallel IO Controller A
extern __weak void PIOA_IrqHandler(void);
// Parallel IO Controller B
extern __weak void PIOB_IrqHandler(void);
// Parallel IO Controller C
extern __weak void PIOC_IrqHandler(void);
// USART 0
extern __weak void USART0_IrqHandler(void);
// USART 1
extern __weak void USART1_IrqHandler(void);
// USART 2
extern __weak void USART2_IrqHandler(void);
// USART 3
extern __weak void USART3_IrqHandler(void);
// Multimedia Card Interface
extern __weak void MCI0_IrqHandler(void);
// TWI 0
extern __weak void TWI0_IrqHandler(void);
// TWI 1
extern __weak void TWI1_IrqHandler(void);
// Serial Peripheral Interface 0
extern __weak void SPI0_IrqHandler(void);
// Serial Synchronous Controller 0
extern __weak void SSC0_IrqHandler(void);
// Timer Counter 0
extern __weak void TC0_IrqHandler(void);
// Timer Counter 1
extern __weak void TC1_IrqHandler(void);
// Timer Counter 2
extern __weak void TC2_IrqHandler(void);
// PWM Controller
extern __weak void PWM_IrqHandler(void);
// ADC controller0
extern __weak void ADCC0_IrqHandler(void);
// ADC controller1
extern __weak void ADCC1_IrqHandler(void);
// HDMA
extern __weak void HDMA_IrqHandler(void);
// USB Device High Speed UDP_HS
extern __weak void UDPD_IrqHandler(void);

#line 37 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\configuration.h"
#line 1 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\drivers\\interrupts.h"
/**********************************************************************************************************************
File: interrupts.h                                                               

Description:
Interrupt declarations for use with MPG development board firmware.
***********************************************************************************************************************/




/***********************************************************************************************************************
Type Definitions
***********************************************************************************************************************/
/// Function prototype for exception table items - interrupt handler.
//typedef void( *IrqHandler )( void );
typedef void( *IntFunc )( void );

typedef enum IRQn
{
/******  Cortex-M3 Processor Exceptions Numbers ***************************************************/
  NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                             */
  MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M3 Memory Management Interrupt              */
  BusFault_IRQn               = -11,    /*!< 5 Cortex-M3 Bus Fault Interrupt                      */
  UsageFault_IRQn             = -10,    /*!< 6 Cortex-M3 Usage Fault Interrupt                    */
  SVCall_IRQn                 = -5,     /*!< 11 Cortex-M3 SV Call Interrupt                       */
  DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M3 Debug Monitor Interrupt                 */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M3 Pend SV Interrupt                       */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M3 System Tick Interrupt                   */

/******  AT91SAM3U4 specific Interrupt Numbers *********************************************************/
 IRQn_SUPC                = ( 0) , // SUPPLY CONTROLLER
 IRQn_RSTC                = ( 1) , // RESET CONTROLLER
 IRQn_RTC                 = ( 2)  , // REAL TIME CLOCK
 IRQn_RTT                 = ( 3)  , // REAL TIME TIMER
 IRQn_WDG                 = ( 4)  , // WATCHDOG TIMER
 IRQn_PMC                 = ( 5)  , // PMC
 IRQn_EFC0                = ( 6) , // EFC0
 IRQn_EFC1                = ( 7) , // EFC1
 IRQn_DBGU                = ( 8) , // DBGU (UART)
 IRQn_HSMC4               = ( 9), // HSMC4
 IRQn_PIOA                = (10) , // Parallel IO Controller A
 IRQn_PIOB                = (11) , // Parallel IO Controller B
 IRQn_PIOC                = (12) , // Parallel IO Controller C
 IRQn_US0                 = (13)  , // USART 0
 IRQn_US1                 = (14)  , // USART 1
 IRQn_US2                 = (15)  , // USART 2
 IRQn_US3                 = (16)  , // USART 3
 IRQn_MCI0                = (17) , // Multimedia Card Interface
 IRQn_TWI0                = (18) , // TWI 0
 IRQn_TWI1                = (19) , // TWI 1
 IRQn_SPI0                = (20) , // Serial Peripheral Interface
 IRQn_SSC0                = (21) , // Serial Synchronous Controller 0
 IRQn_TC0                 = (22)  , // Timer Counter 0
 IRQn_TC1                 = (23)  , // Timer Counter 1
 IRQn_TC2                 = (24)  , // Timer Counter 2
 IRQn_PWMC                = (25) , // Pulse Width Modulation Controller
 IRQn_ADCC0               = (26), // ADC controller0
 IRQn_ADCC1               = (27)  , // ADC controller1
 IRQn_HDMA                = (28) , // HDMA
 IRQn_UDPHS               = (29)  // USB Device High Speed
} IRQn_Type;


/***********************************************************************************************************************
Constants / Definitions
***********************************************************************************************************************/
/* IRQ Channels -> listed in AT91SAM3U4.h around line 6686 */




/**********************************************************************************************************************
Function Prototypes
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/





/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
#line 38 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\configuration.h"
#line 1 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"
/******************************************************************************
 * @file:    core_cm3.h
 * @purpose: CMSIS Cortex-M3 Core Peripheral Access Layer Header File
 * @version: V1.10
 * @date:    24. Feb. 2009
 *----------------------------------------------------------------------------
 *
 * Copyright (C) 2009 ARM Limited. All rights reserved.
 *
 * ARM Limited (ARM) is supplying this software for use with Cortex-Mx 
 * processor based microcontrollers.  This file can be freely distributed 
 * within development tools that are supporting such ARM based processors. 
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/














/**
 *  Lint configuration \n
 *  ----------------------- \n
 *
 *  The following Lint messages will be suppressed and not shown: \n
 *  \n
 *    --- Error 10: --- \n
 *    register uint32_t __regBasePri         __asm("basepri"); \n
 *    Error 10: Expecting ';' \n
 *     \n
 *    --- Error 530: --- \n
 *    return(__regBasePri); \n
 *    Warning 530: Symbol '__regBasePri' (line 264) not initialized \n
 *     \n
 *    --- Error 550: --- \n
 *      __regBasePri = (basePri & 0x1ff); \n
 *    } \n
 *    Warning 550: Symbol '__regBasePri' (line 271) not accessed \n
 *     \n
 *    --- Error 754: --- \n
 *    uint32_t RESERVED0[24]; \n
 *    Info 754: local structure member '<some, not used in the HAL>' (line 109, file ./cm3_core.h) not referenced \n
 *     \n
 *    --- Error 750: --- \n
 *    #define __CM3_CORE_H__ \n
 *    Info 750: local macro '__CM3_CORE_H__' (line 43, file./cm3_core.h) not referenced \n
 *     \n
 *    --- Error 528: --- \n
 *    static __INLINE void NVIC_DisableIRQ(uint32_t IRQn) \n
 *    Warning 528: Symbol 'NVIC_DisableIRQ(unsigned int)' (line 419, file ./cm3_core.h) not referenced \n
 *     \n
 *    --- Error 751: --- \n
 *    } InterruptType_Type; \n
 *    Info 751: local typedef 'InterruptType_Type' (line 170, file ./cm3_core.h) not referenced \n
 * \n
 * \n
 *    Note:  To re-enable a Message, insert a space before 'lint' * \n
 *
 */

/*lint -save */
/*lint -e10  */
/*lint -e530 */
/*lint -e550 */
/*lint -e754 */
/*lint -e750 */
/*lint -e528 */
/*lint -e751 */


#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdint.h"
/* stdint.h standard header */
/* Copyright 2003-2010 IAR Systems AB.  */




  #pragma system_include


#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"
/* ycheck.h internal checking header file. */
/* Copyright 2005-2010 IAR Systems AB. */

/* Note that there is no include guard for this header. This is intentional. */


  #pragma system_include


/* __INTRINSIC
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that intrinsic support could be turned off
 * individually for each file.
 */










/* __AEABI_PORTABILITY_INTERNAL_LEVEL
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that ABI support could be turned off/on
 * individually for each file.
 *
 * Possible values for this preprocessor symbol:
 *
 * 0 - ABI portability mode is disabled.
 *
 * 1 - ABI portability mode (version 1) is enabled.
 *
 * All other values are reserved for future use.
 */






#line 67 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"

#line 11 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdint.h"



/* Fixed size types. These are all optional. */

  typedef signed char   int8_t;
  typedef unsigned char uint8_t;



  typedef signed short int   int16_t;
  typedef unsigned short int uint16_t;



  typedef signed int   int32_t;
  typedef unsigned int uint32_t;



  #pragma language=save
  #pragma language=extended
  typedef signed long long int   int64_t;
  typedef unsigned long long int uint64_t;
  #pragma language=restore


/* Types capable of holding at least a certain number of bits.
   These are not optional for the sizes 8, 16, 32, 64. */
typedef signed char   int_least8_t;
typedef unsigned char uint_least8_t;

typedef signed short int   int_least16_t;
typedef unsigned short int uint_least16_t;

typedef signed int   int_least32_t;
typedef unsigned int uint_least32_t;

/* This isn't really optional, but make it so for now. */

  #pragma language=save
  #pragma language=extended
  typedef signed long long int int_least64_t;
  #pragma language=restore


  #pragma language=save
  #pragma language=extended
  typedef unsigned long long int uint_least64_t;
  #pragma language=restore


/* The fastest type holding at least a certain number of bits.
   These are not optional for the size 8, 16, 32, 64.
   For now, the 64 bit size is optional in IAR compilers. */
typedef signed int   int_fast8_t;
typedef unsigned int uint_fast8_t;

typedef signed int   int_fast16_t;
typedef unsigned int uint_fast16_t;

typedef signed int   int_fast32_t;
typedef unsigned int uint_fast32_t;


  #pragma language=save
  #pragma language=extended
  typedef signed long long int int_fast64_t;
  #pragma language=restore


  #pragma language=save
  #pragma language=extended
  typedef unsigned long long int uint_fast64_t;
  #pragma language=restore


/* The integer type capable of holding the largest number of bits. */
#pragma language=save
#pragma language=extended
typedef signed long long int   intmax_t;
typedef unsigned long long int uintmax_t;
#pragma language=restore

/* An integer type large enough to be able to hold a pointer.
   This is optional, but always supported in IAR compilers. */
typedef signed long int   intptr_t;
typedef unsigned long int uintptr_t;

/* An integer capable of holding a pointer to a specific memory type. */



typedef int __data_intptr_t; typedef unsigned int __data_uintptr_t;


/* Minimum and maximum limits. */






























































































/* Macros expanding to integer constants. */

































#line 258 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\stdint.h"

/*
 * Copyright (c) 1992-2009 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.04:0576 */
#line 86 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"

#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\intrinsics.h"
/**************************************************
 *
 * This file declares the ARM intrinsic inline functions.
 *
 * Copyright 1999-2006 IAR Systems. All rights reserved.
 *
 * $Revision: 65584 $
 *
 **************************************************/




#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"
/* ycheck.h internal checking header file. */
/* Copyright 2005-2010 IAR Systems AB. */

/* Note that there is no include guard for this header. This is intentional. */


  #pragma system_include


/* __INTRINSIC
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that intrinsic support could be turned off
 * individually for each file.
 */










/* __AEABI_PORTABILITY_INTERNAL_LEVEL
 *
 * Note: Redefined each time ycheck.h is included, i.e. for each
 * system header, to ensure that ABI support could be turned off/on
 * individually for each file.
 *
 * Possible values for this preprocessor symbol:
 *
 * 0 - ABI portability mode is disabled.
 *
 * 1 - ABI portability mode (version 1) is enabled.
 *
 * All other values are reserved for future use.
 */






#line 67 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\ycheck.h"

#line 15 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\intrinsics.h"


  #pragma system_include


/*
 * Check that the correct C compiler is used.
 */









#pragma language=save
#pragma language=extended

__intrinsic __nounwind void    __no_operation(void);

__intrinsic __nounwind void    __disable_interrupt(void);
__intrinsic __nounwind void    __enable_interrupt(void);

typedef unsigned long __istate_t;

__intrinsic __nounwind __istate_t __get_interrupt_state(void);
__intrinsic __nounwind void __set_interrupt_state(__istate_t);



/* System control access for Cortex-M cores */
__intrinsic __nounwind unsigned long __get_PSR( void );
__intrinsic __nounwind unsigned long __get_IPSR( void );
__intrinsic __nounwind unsigned long __get_MSP( void );
__intrinsic __nounwind void          __set_MSP( unsigned long );
__intrinsic __nounwind unsigned long __get_PSP( void );
__intrinsic __nounwind void          __set_PSP( unsigned long );
__intrinsic __nounwind unsigned long __get_PRIMASK( void );
__intrinsic __nounwind void          __set_PRIMASK( unsigned long );
__intrinsic __nounwind unsigned long __get_CONTROL( void );
__intrinsic __nounwind void          __set_CONTROL( unsigned long );



/* These are only available for v7M */
__intrinsic __nounwind unsigned long __get_FAULTMASK( void );
__intrinsic __nounwind void          __set_FAULTMASK(unsigned long);
__intrinsic __nounwind unsigned long __get_BASEPRI( void );
__intrinsic __nounwind void          __set_BASEPRI( unsigned long );

#line 74 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\intrinsics.h"

__intrinsic __nounwind void __disable_fiq(void);
__intrinsic __nounwind void __enable_fiq(void);


/* ARM-mode intrinsics */

__intrinsic __nounwind unsigned long __SWP( unsigned long, volatile unsigned long * );
__intrinsic __nounwind unsigned char __SWPB( unsigned char, volatile unsigned char * );

typedef unsigned long __ul;





/*  Co-processor access */
__intrinsic __nounwind void          __MCR( unsigned __constrange(0,15) coproc, unsigned __constrange(0,8) opcode_1, __ul src,
                                 unsigned __constrange(0,15) CRn, unsigned __constrange(0,15) CRm, unsigned __constrange(0,8) opcode_2 );
__intrinsic __nounwind unsigned long __MRC( unsigned __constrange(0,15) coproc, unsigned __constrange(0,8) opcode_1,
                                 unsigned __constrange(0,15) CRn, unsigned __constrange(0,15) CRm, unsigned __constrange(0,8) opcode_2 );
__intrinsic __nounwind void          __MCR2( unsigned __constrange(0,15) coproc, unsigned __constrange(0,8) opcode_1, __ul src,
                                  unsigned __constrange(0,15) CRn, unsigned __constrange(0,15) CRm, unsigned __constrange(0,8) opcode_2 );
__intrinsic __nounwind unsigned long __MRC2( unsigned __constrange(0,15) coproc, unsigned __constrange(0,8) opcode_1,
                                  unsigned __constrange(0,15) CRn, unsigned __constrange(0,15) CRm, unsigned __constrange(0,8) opcode_2 );

/* Load coprocessor register. */
__intrinsic __nounwind void __LDC( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul const *src);
__intrinsic __nounwind void __LDCL( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul const *src);
__intrinsic __nounwind void __LDC2( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul const *src);
__intrinsic __nounwind void __LDC2L( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul const *src);

/* Store coprocessor register. */
__intrinsic __nounwind void __STC( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul *dst);
__intrinsic __nounwind void __STCL( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul *dst);
__intrinsic __nounwind void __STC2( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul *dst);
__intrinsic __nounwind void __STC2L( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul *dst);

/* Load coprocessor register (noindexed version with coprocessor option). */
__intrinsic __nounwind void __LDC_noidx( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul const *src,
                              unsigned __constrange(0,255) option);

__intrinsic __nounwind void __LDCL_noidx( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul const *src,
                               unsigned __constrange(0,255) option);

__intrinsic __nounwind void __LDC2_noidx( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul const *src,
                               unsigned __constrange(0,255) option);

__intrinsic __nounwind void __LDC2L_noidx( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul const *src,
                                unsigned __constrange(0,255) option);

/* Store coprocessor register (version with coprocessor option). */
__intrinsic __nounwind void __STC_noidx( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul *dst,
                              unsigned __constrange(0,255) option);

__intrinsic __nounwind void __STCL_noidx( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul *dst,
                               unsigned __constrange(0,255) option);

__intrinsic __nounwind void __STC2_noidx( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul *dst,
                               unsigned __constrange(0,255) option);

__intrinsic __nounwind void __STC2L_noidx( unsigned __constrange(0,15) coproc, unsigned __constrange(0,15) CRn, volatile __ul *dst,
                                unsigned __constrange(0,255) option);


/* Status register access, v7M: */
__intrinsic __nounwind unsigned long __get_APSR( void );
__intrinsic __nounwind void          __set_APSR( unsigned long );






/* Floating-point status and control register access */
__intrinsic __nounwind unsigned long __get_FPSCR( void );
__intrinsic __nounwind void __set_FPSCR( unsigned long );

/* Architecture v5T, CLZ is also available in Thumb mode for Thumb2 cores */
__intrinsic __nounwind unsigned char __CLZ( unsigned long );

/* Architecture v5TE */
#line 167 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\intrinsics.h"

__intrinsic __nounwind int         __QCFlag( void );
__intrinsic __nounwind void __reset_QC_flag( void );

__intrinsic __nounwind signed long __SMUL( signed short, signed short );

/* Architecture v6, REV and REVSH are also available in thumb mode */
__intrinsic __nounwind unsigned long __REV( unsigned long );
__intrinsic __nounwind signed long __REVSH( short );

__intrinsic __nounwind unsigned long __REV16( unsigned long );
__intrinsic __nounwind unsigned long __RBIT( unsigned long );

__intrinsic __nounwind unsigned char  __LDREXB( volatile unsigned char const * );
__intrinsic __nounwind unsigned short __LDREXH( volatile unsigned short const * );
__intrinsic __nounwind unsigned long  __LDREX ( volatile unsigned long const * );
__intrinsic __nounwind unsigned long long __LDREXD( volatile unsigned long long const * );

__intrinsic __nounwind unsigned long  __STREXB( unsigned char, volatile unsigned char * );
__intrinsic __nounwind unsigned long  __STREXH( unsigned short, volatile unsigned short * );
__intrinsic __nounwind unsigned long  __STREX ( unsigned long, volatile unsigned long * );
__intrinsic __nounwind unsigned long  __STREXD( unsigned long long, volatile unsigned long long * );

__intrinsic __nounwind void __CLREX( void );

__intrinsic __nounwind void __SEV( void );
__intrinsic __nounwind void __WFE( void );
__intrinsic __nounwind void __WFI( void );
__intrinsic __nounwind void __YIELD( void );

__intrinsic __nounwind void __PLI( volatile void const * );
__intrinsic __nounwind void __PLD( volatile void const * );
__intrinsic __nounwind void __PLDW( volatile void const * );

__intrinsic __nounwind   signed long __SSAT     (unsigned long val,
                                      unsigned int __constrange( 1, 32 ) sat );
__intrinsic __nounwind unsigned long __USAT     (unsigned long val,
                                      unsigned int __constrange( 0, 31 ) sat );


#line 374 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\intrinsics.h"

/* Architecture v7 instructions.... */
__intrinsic __nounwind void __DMB(void);
__intrinsic __nounwind void __DSB(void);
__intrinsic __nounwind void __ISB(void);




#pragma language=restore





#line 89 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"










/**
 * IO definitions
 *
 * define access restrictions to peripheral registers
 */







/*******************************************************************************
 *                 Register Abstraction
 ******************************************************************************/


/* System Reset */





/* Core Debug */






/* memory mapping struct for Nested Vectored Interrupt Controller (NVIC) */
typedef struct
{
  volatile uint32_t ISER[8];                      /*!< Interrupt Set Enable Register            */
       uint32_t RESERVED0[24];
  volatile uint32_t ICER[8];                      /*!< Interrupt Clear Enable Register          */
       uint32_t RSERVED1[24];
  volatile uint32_t ISPR[8];                      /*!< Interrupt Set Pending Register           */
       uint32_t RESERVED2[24];
  volatile uint32_t ICPR[8];                      /*!< Interrupt Clear Pending Register         */
       uint32_t RESERVED3[24];
  volatile uint32_t IABR[8];                      /*!< Interrupt Active bit Register            */
       uint32_t RESERVED4[56];
  volatile uint8_t  IP[240];                      /*!< Interrupt Priority Register, 8Bit wide   */
       uint32_t RESERVED5[644];
  volatile  uint32_t STIR;                         /*!< Software Trigger Interrupt Register      */
}  NVIC_Type;


/* memory mapping struct for System Control Block */
typedef struct
{
  volatile const  uint32_t CPUID;                        /*!< CPU ID Base Register                                     */
  volatile uint32_t ICSR;                         /*!< Interrupt Control State Register                         */
  volatile uint32_t VTOR;                         /*!< Vector Table Offset Register                             */
  volatile uint32_t AIRCR;                        /*!< Application Interrupt / Reset Control Register           */
  volatile uint32_t SCR;                          /*!< System Control Register                                  */
  volatile uint32_t CCR;                          /*!< Configuration Control Register                           */
  volatile uint8_t  SHP[12];                      /*!< System Handlers Priority Registers (4-7, 8-11, 12-15)    */
  volatile uint32_t SHCSR;                        /*!< System Handler Control and State Register                */
  volatile uint32_t CFSR;                         /*!< Configurable Fault Status Register                       */
  volatile uint32_t HFSR;                         /*!< Hard Fault Status Register                               */
  volatile uint32_t DFSR;                         /*!< Debug Fault Status Register                              */
  volatile uint32_t MMFAR;                        /*!< Mem Manage Address Register                              */
  volatile uint32_t BFAR;                         /*!< Bus Fault Address Register                               */
  volatile uint32_t AFSR;                         /*!< Auxiliary Fault Status Register                          */
  volatile const  uint32_t PFR[2];                       /*!< Processor Feature Register                               */
  volatile const  uint32_t DFR;                          /*!< Debug Feature Register                                   */
  volatile const  uint32_t ADR;                          /*!< Auxiliary Feature Register                               */
  volatile const  uint32_t MMFR[4];                      /*!< Memory Model Feature Register                            */
  volatile const  uint32_t ISAR[5];                      /*!< ISA Feature Register                                     */
} SCB_Type;


/* memory mapping struct for SysTick */
typedef struct
{
  volatile uint32_t CTRL;                         /*!< SysTick Control and Status Register */
  volatile uint32_t LOAD;                         /*!< SysTick Reload Value Register       */
  volatile uint32_t VAL;                          /*!< SysTick Current Value Register      */
  volatile const  uint32_t CALIB;                        /*!< SysTick Calibration Register        */
} SysTick_Type;


/* memory mapping structur for ITM */
typedef struct
{
  volatile  union  
  {
    volatile  uint8_t    u8;                       /*!< ITM Stimulus Port 8-bit               */
    volatile  uint16_t   u16;                      /*!< ITM Stimulus Port 16-bit              */
    volatile  uint32_t   u32;                      /*!< ITM Stimulus Port 32-bit              */
  }  PORT [32];                               /*!< ITM Stimulus Port Registers           */
       uint32_t RESERVED0[864];
  volatile uint32_t TER;                          /*!< ITM Trace Enable Register             */
       uint32_t RESERVED1[15];
  volatile uint32_t TPR;                          /*!< ITM Trace Privilege Register          */
       uint32_t RESERVED2[15];
  volatile uint32_t TCR;                          /*!< ITM Trace Control Register            */
       uint32_t RESERVED3[29];
  volatile uint32_t IWR;                          /*!< ITM Integration Write Register        */
  volatile uint32_t IRR;                          /*!< ITM Integration Read Register         */
  volatile uint32_t IMCR;                         /*!< ITM Integration Mode Control Register */
       uint32_t RESERVED4[43];
  volatile uint32_t LAR;                          /*!< ITM Lock Access Register              */
  volatile uint32_t LSR;                          /*!< ITM Lock Status Register              */
       uint32_t RESERVED5[6];
  volatile const  uint32_t PID4;                         /*!< ITM Product ID Registers              */
  volatile const  uint32_t PID5;
  volatile const  uint32_t PID6;
  volatile const  uint32_t PID7;
  volatile const  uint32_t PID0;
  volatile const  uint32_t PID1;
  volatile const  uint32_t PID2;
  volatile const  uint32_t PID3;
  volatile const  uint32_t CID0;
  volatile const  uint32_t CID1;
  volatile const  uint32_t CID2;
  volatile const  uint32_t CID3;
} ITM_Type;


/* memory mapped struct for Interrupt Type */
typedef struct
{
       uint32_t RESERVED0;
  volatile const  uint32_t ICTR;                         /*!< Interrupt Control Type Register  */



       uint32_t RESERVED1;

} InterruptType_Type;


/* Memory Protection Unit */
#line 251 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"


/* Core Debug Register */
typedef struct
{
  volatile uint32_t DHCSR;                        /*!< Debug Halting Control and Status Register       */
  volatile  uint32_t DCRSR;                        /*!< Debug Core Register Selector Register           */
  volatile uint32_t DCRDR;                        /*!< Debug Core Register Data Register               */
  volatile uint32_t DEMCR;                        /*!< Debug Exception and Monitor Control Register    */
} CoreDebug_Type;


/* Memory mapping of Cortex-M3 Hardware */
#line 270 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"

#line 277 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"








/*******************************************************************************
 *                Hardware Abstraction Layer
 ******************************************************************************/


#line 298 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"








/* ###################  Compiler specific Intrinsics  ########################### */

#line 623 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"
/* IAR iccarm specific functions */



#line 1 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\cmsis_iar.h"
/**************************************************
 *
 * This file shall be included in appropriate CMSIS header
 * files, to provide required functions and intrinsics when
 * building with the IAR C/C++ Compiler for ARM (iccarm).
 *
 * Copyright 2011 IAR Systems. All rights reserved.
 *
 * $Revision: 78346 $
 *
 **************************************************/








#pragma system_include









#pragma diag_suppress=Pe940
#pragma diag_suppress=Pe177











#line 110 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\cmsis_iar.h"

static uint32_t __get_xPSR(void)
{
  return __get_PSR();   /* __get_PSR() intrinsic introduced in iccarm 6.20 */
}





#line 173 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\cmsis_iar.h"










#line 206 "C:\\Program Files (x86)\\IAR Systems\\EWARM_7_20_1\\arm\\inc\\c\\cmsis_iar.h"





static inline uint32_t __RRX(uint32_t value)
{
  uint32_t result;
  __asm("RRX %0, %1" : "=r"(result) : "r" (value) );
  return(result);
}

static inline uint8_t __LDRBT(volatile uint8_t *addr)
{
  uint32_t result;
  __asm("LDRBT %0, [%1]" : "=r" (result) : "r" (addr) : "memory" );
  return ((uint8_t) result);
}

static inline uint16_t __LDRHT(volatile uint16_t *addr)
{
  uint32_t result;
  __asm("LDRHT %0, [%1]" : "=r" (result) : "r" (addr) : "memory" );
  return ((uint16_t) result);
}

static inline uint32_t __LDRT(volatile uint32_t *addr)
{
  uint32_t result;
  __asm("LDRT %0, [%1]" : "=r" (result) : "r" (addr) : "memory" );
  return(result);
}

static inline void __STRBT(uint8_t value, volatile uint8_t *addr)
{
  __asm("STRBT %1, [%0]" : : "r" (addr), "r" ((uint32_t)value) : "memory" );
}

static inline void __STRHT(uint16_t value, volatile uint16_t *addr)
{
  __asm("STRHT %1, [%0]" : : "r" (addr), "r" ((uint32_t)value) : "memory" );
}

static inline void __STRT(uint32_t value, volatile uint32_t *addr)
{
  __asm("STRT %1, [%0]" : : "r" (addr), "r" (value) : "memory" );
}





static inline uint32_t __ROR(uint32_t op1, uint32_t op2)
{
  return (op1 >> op2) | (op1 << ((sizeof(op1)*8)-op2));
}

#pragma diag_default=Pe940
#pragma diag_default=Pe177

#line 628 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"

#line 799 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"

#line 1048 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\cmsis\\core_cm3.h"



/* ##########################   NVIC functions  #################################### */

/**
 * @brief  Set the Priority Grouping in NVIC Interrupt Controller
 *
 * @param  uint32_t priority_grouping is priority grouping field
 * @return  
 *
 * Set the priority grouping field using the required unlock sequence.
 * The parameter priority_grouping is assigned to the field 
 * SCB->AIRCR [10:8] PRIGROUP field.
 */
static inline void NVIC_SetPriorityGrouping(uint32_t priority_grouping)
{
  uint32_t reg_value=0;
  
  reg_value  = ((SCB_Type *) ((0xE000E000) + 0x0D00))->AIRCR;                                                                            /* read old register configuration    */
  reg_value &= ~((0xFFFFU << 16) | (0x0F << 8));                                                      /* clear bits to change               */
  reg_value  = ((reg_value | (0x5FA << 16) | (priority_grouping << 8)));                         /* Insert write key and priorty group */
  ((SCB_Type *) ((0xE000E000) + 0x0D00))->AIRCR = reg_value;
}

/**
 * @brief  Enable Interrupt in NVIC Interrupt Controller
 *
 * @param  IRQn_Type IRQn specifies the interrupt number
 * @return none 
 *
 * Enable a device specific interupt in the NVIC interrupt controller.
 * The interrupt number cannot be a negative value.
 */
static inline void NVIC_EnableIRQ(IRQn_Type IRQn)
{
  ((NVIC_Type *) ((0xE000E000) + 0x0100))->ISER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));                             /* enable interrupt */
}

/**
 * @brief  Disable the interrupt line for external interrupt specified
 * 
 * @param  IRQn_Type IRQn is the positive number of the external interrupt
 * @return  none
 * 
 * Disable a device specific interupt in the NVIC interrupt controller.
 * The interrupt number cannot be a negative value.
 */
static inline void NVIC_DisableIRQ(IRQn_Type IRQn)
{
  ((NVIC_Type *) ((0xE000E000) + 0x0100))->ICER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));                             /* disable interrupt */
}

/**
 * @brief  Read the interrupt pending bit for a device specific interrupt source
 * 
 * @param  IRQn_Type IRQn is the number of the device specifc interrupt
 * @return IRQn_Type Number of pending interrupt or zero
 *
 * Read the pending register in NVIC and return the number of the
 * specified interrupt if its status is pending, otherwise it returns 
 * zero. The interrupt number cannot be a negative value.
 */
static inline IRQn_Type NVIC_GetPendingIRQ(IRQn_Type IRQn)
{
  return((IRQn_Type) (((NVIC_Type *) ((0xE000E000) + 0x0100))->ISPR[(uint32_t)(IRQn) >> 5] & (1 << ((uint32_t)(IRQn) & 0x1F))));         /* Return Interrupt bit or 'zero' */
}

/**
 * @brief  Set the pending bit for an external interrupt
 * 
 * @param  IRQn_Type IRQn is the Number of the interrupt
 * @return  none
 *
 * Set the pending bit for the specified interrupt.
 * The interrupt number cannot be a negative value.
 */
static inline void NVIC_SetPendingIRQ(IRQn_Type IRQn)
{
  ((NVIC_Type *) ((0xE000E000) + 0x0100))->ISPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));                             /* set interrupt pending */
}

/**
 * @brief  Clear the pending bit for an external interrupt
 *
 * @param  IRQn_Type IRQn is the Number of the interrupt
 * @return  none
 *
 * Clear the pending bit for the specified interrupt. 
 * The interrupt number cannot be a negative value.
 */
static inline void NVIC_ClearPendingIRQ(IRQn_Type IRQn)
{
  ((NVIC_Type *) ((0xE000E000) + 0x0100))->ICPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));                             /* Clear pending interrupt */
}

/**
 * @brief  Read the active bit for an external interrupt
 *
 * @param  IRQn_Type IRQn is the Number of the interrupt
 * @return IRQn_Type Number of pending interrupt or zero
 *
 * Read the active register in NVIC and returns the number of the
 * specified interrupt if its status is active, otherwise it 
 * returns zero. The interrupt number cannot be a negative value.
 */
static inline IRQn_Type NVIC_GetActive(IRQn_Type IRQn)
{
  return((IRQn_Type)(((NVIC_Type *) ((0xE000E000) + 0x0100))->IABR[(uint32_t)(IRQn) >> 5] & (1 << ((uint32_t)(IRQn) & 0x1F))));                        /* Return Interruptnumber or 'zero' */
}

/**
 * @brief  Set the priority for an interrupt
 *
 * @param  IRQn_Type IRQn is the Number of the interrupt
 * @param  priority is the priority for the interrupt
 * @return  none
 *
 * Set the priority for the specified interrupt. The interrupt 
 * number can be positive to specify an external (device specific) 
 * interrupt, or negative to specify an internal (core) interrupt. \n
 *
 * Note: The priority cannot be set for every core interrupt.
 */
static inline void NVIC_SetPriority(IRQn_Type IRQn, int32_t priority)
{
  if(IRQn < 0) {
    ((SCB_Type *) ((0xE000E000) + 0x0D00))->SHP[((uint32_t)(IRQn) & 0xF)-4] = ((priority << (8 - 4)) & 0xff); }  /* set Priority for Cortex-M3 System Interrupts */
  else {
    //NVIC->IP[(uint32_t)(IRQn)] = ((priority << (8 - __NVIC_PRIO_BITS)) & 0xff);    }         /* set Priority for device specific Interrupts      */
    ((NVIC_Type *) ((0xE000E000) + 0x0100))->IP[(uint32_t)(IRQn)] = (priority & 0xff);    }         /* set Priority for device specific Interrupts      */
}

/**
 * @brief  Read the priority for an interrupt
 *
 * @param  IRQn_Type IRQn is the Number of the interrupt
 * @return priority is the priority for the interrupt
 *
 * Read the priority for the specified interrupt. The interrupt 
 * number can be positive to specify an external (device specific) 
 * interrupt, or negative to specify an internal (core) interrupt.
 *
 * The returned priority value is automatically aligned to the implemented
 * priority bits of the microcontroller.
 *
 * Note: The priority cannot be set for every core interrupt.
 */
static inline uint32_t NVIC_GetPriority(IRQn_Type IRQn)
{

  if(IRQn < 0) {
    return((uint32_t)(((SCB_Type *) ((0xE000E000) + 0x0D00))->SHP[((uint32_t)(IRQn) & 0xF)-4] >> (8 - 4)));  }              /* get priority for Cortex-M3 system interrupts   */
  else {
    return((uint32_t)(((NVIC_Type *) ((0xE000E000) + 0x0100))->IP[(uint32_t)(IRQn)]           >> (8 - 4)));  }              /* get priority for device specific interrupts    */
}



/* ##################################    SysTick function  ############################################ */



/* SysTick constants */





/**
 * @brief  Initialize and start the SysTick counter and its interrupt.
 *
 * @param  uint32_t ticks is the number of ticks between two interrupts
 * @return  none
 *
 * Initialise the system tick timer and its interrupt and start the
 * system tick timer / counter in free running mode to generate 
 * periodical interrupts.
 */
static inline uint32_t SysTick_Config(uint32_t ticks)
{ 
  if (ticks > ((1<<24) -1))  return (1);                                                /* Reload value impossible */

  ((SysTick_Type *) ((0xE000E000) + 0x0010))->LOAD  =  (ticks & ((1<<24) -1)) - 1;                                         /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1<<4) - 1);                               /* set Priority for Cortex-M0 System Interrupts */
  ((SysTick_Type *) ((0xE000E000) + 0x0010))->VAL   =  (0x00);                                                                 /* Load the SysTick Counter Value */
  ((SysTick_Type *) ((0xE000E000) + 0x0010))->CTRL = (1 << 2) | (1<<0) | (1<<1);    /* Enable SysTick IRQ and SysTick Timer */
  return (0);                                                                               /* Function successful */
}







/* ##################################    Reset function  ############################################ */

/**
 * @brief  Initiate a system reset request.
 *
 * @param   none
 * @return  none
 *
 * Initialize a system reset request to reset the MCU
 */
static inline void NVIC_SystemReset(void)
{
  ((SCB_Type *) ((0xE000E000) + 0x0D00))->AIRCR  = ((0x5FA << 16) | (((SCB_Type *) ((0xE000E000) + 0x0D00))->AIRCR & (0x700)) | (1<<2));      /* Keep priority group unchanged */
}


/* ##################################    Debug Output  function  ############################################ */


/**
 * @brief  Outputs a character via the ITM channel 0
 *
 * @param   uint32_t character to output
 * @return  uint32_t input character
 *
 * The function outputs a character via the ITM channel 0. 
 * The function returns when no debugger is connected that has booked the output.  
 * It is blocking when a debugger is connected, but the previous character send is not transmitted. 
 */
static inline uint32_t ITM_SendChar (uint32_t ch)
{
  if(ch == '\n') ITM_SendChar('\r');
  
  if ((((CoreDebug_Type *) (0xE000EDF0))->DEMCR & (1 << 24))  &&
      (((ITM_Type *) (0xE0000000))->TCR & 1)                  &&
      (((ITM_Type *) (0xE0000000))->TER & (1UL << 0))  ) 
  {
    while (((ITM_Type *) (0xE0000000))->PORT[0].u32 == 0);
    ((ITM_Type *) (0xE0000000))->PORT[0].u8 = (uint8_t) ch;
  }  
  return (ch);
}



/*lint -restore */
#line 39 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\configuration.h"
#line 1 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\typedefs.h"
/*******************************************************************************
* File: typedefs.h                                                               
* Description:
* Type definitions for use with the system.
*******************************************************************************/





typedef void(*fnCode_type)(void);

/* CHAR/SHORT/LONG types here for legacy code compatibility */
typedef char CHAR;              /* Signed 8-bits */
typedef unsigned char UCHAR;    /* Unsigned 8-bits */
typedef short SHORT;            /* Signed 16-bits */
typedef unsigned short USHORT;  /* Unsigned 16-bits */
typedef long LONG;              /* Signed 32-bits */
typedef unsigned long ULONG;    /* Unsigned 32-bits */
typedef unsigned char BOOL;     /* Boolean */


/* Standard Peripheral Library old types (maintained for legacy purpose) */
typedef long s32;
typedef short s16;
typedef signed char  s8;

typedef const long sc32;  /*!< Read Only */
typedef const short sc16;  /*!< Read Only */
typedef const char sc8;   /*!< Read Only */

typedef ULONG  u32;
typedef USHORT u16;
typedef UCHAR  u8;

typedef const ULONG uc32;  /*!< Read Only */
typedef const USHORT uc16;  /*!< Read Only */
typedef const USHORT uc8;   /*!< Read Only */



typedef enum {FALSE = 0, TRUE = !FALSE} bool;


typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;


typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;

#line 84 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\typedefs.h"




#line 40 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\configuration.h"

#line 1 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\application\\main.h"
/***********************************************************************************************************************
File: main.h

Description:
Header file for main.c.
*******************************************************************************/




/***********************************************************************************************************************
* Firmware Version - Printed FIRMWARE_MAIN_REV.FIRMWARE_SUB_REV1 FIRMWARE_SUB_REV2
* See releasenotes.txt for firmware details.
***********************************************************************************************************************/





/***********************************************************************************************************************
* Constant Definitions
***********************************************************************************************************************/
/* G_u32ApplicationFlags definitions are in configuration.h */

/* G_u32SystemFlags */

#line 42 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\configuration.h"
#line 1 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\drivers\\utilities.h"
/***********************************************************************************************************************
File: utilities.h                                                                
***********************************************************************************************************************/




/***********************************************************************************************************************
Type Definitions
***********************************************************************************************************************/


/***********************************************************************************************************************
Constants / Definitions
***********************************************************************************************************************/



























/***********************************************************************************************************************
* Function Declarations
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions */
/*--------------------------------------------------------------------------------------------------------------------*/
u8 ASCIIHexCharToChar(u8);
u8 HexToASCIICharUpper(u8 u8Char_);
u8 HexToASCIICharLower(u8 u8Char_);
u8 NumberToAscii(u32 u32Number_, u8* pu8AsciiString_);
bool SearchString(u8* pu8TargetString_, u8* pu8MatchString_);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/





/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
#line 43 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpgl1\\iar_7_20_1\\..\\..\\firmware_mpg_common\\configuration.h"


/**********************************************************************************************************************
!!!!! External peripheral assignments
***********************************************************************************************************************/


/***********************************************************************************************************************
@@@@@ GPIO board-specific parameters
***********************************************************************************************************************/


/***********************************************************************************************************************
##### Communication peripheral board-specific parameters
***********************************************************************************************************************/



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

#line 23 "D:\\MPG Source\\Git\\Razor_Atmel\\firmware_mpg_common\\drivers\\utilities.c"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
u8 G_au8MessageOK[]   = "OK\r\n";            /* Common "OK" message */
u8 G_au8MessageFAIL[] = "FAIL\r\n";          /* Common "FAIL" message */
u8 G_au8MessageON[]   = "ON\r\n";            /* Common "ON" message */
u8 G_au8MessageOFF[]  = "OFF\r\n";           /* Common "OFF" message */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;        /* From scroll-ehw-ae.c */
extern volatile u32 G_u32SystemTime1s;         /* From scroll-ehw-ae.c */

extern volatile u32 G_u32SystemFlags;          /* From main.c */
extern volatile u32 G_u32ApplicationFlags;     /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Util_" and be declared as static.
***********************************************************************************************************************/


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------/
Function: ASCIIHexCharToChar

Description:
Determines the numerical value of a hexidecimal ASCII char of that number
('0' - 'F' or '0' - 'f' -> 0 - 15).

Requires:
  - Standard ASCII table is in use
 
Promises:
  - If valid 0-9, A-F or a-f, returns the numerical value of the ASCII char
  - Otherwise returns 0xff
*/
u8 ASCIIHexCharToChar(u8 u8Char_)
{
  if( (u8Char_ >= '0') && (u8Char_ <= '9') )
    return(u8Char_ - (u8)48);
       
  if( (u8Char_ >= 'A') && (u8Char_ <= 'F') )
    return(u8Char_ - (u8)55);
  
  if( (u8Char_ >= 'a') && (u8Char_ <= 'f') )
    return(u8Char_ - (u8)87);
  
  return(0xff);

} /* end ASCIIHexChartoChar */


/*-----------------------------------------------------------------------------/
Function: HexToASCIICharUpper

Description:
Determines the ASCII char of a single digit number
0 - 15 -> '0' - 'F'

Requires:
  - Standard ASCII table is in use
 
Promises:
  - If valid 0-15, returns the corresponding ASCII hex char 0-9, A-F
  - Otherwise returns 0xff
*/
u8 HexToASCIICharUpper(u8 u8Char_)
{
  u8 u8ReturnValue = 0xFF;
  
  if( u8Char_ <= 9 )
  {
    u8ReturnValue = u8Char_ + (u8)48;
  }
  
  else if( (u8Char_ >= 10) && (u8Char_ <= 15) )
  {
    u8ReturnValue = u8Char_ + (u8)55;
  }
  
  return(u8ReturnValue);

} /* end HexToASCIICharUpper */


/*-----------------------------------------------------------------------------/
Function: HexToASCIICharLower

Description:
Determines the ASCII char of a single digit number
0 - 15 -> '0' - 'f'

Requires:
  - Standard ASCII table is in use
 
Promises:
  - If valid 0-15, returns the corresponding ASCII hex char 0-9, a-f
  - Otherwise returns 0xFF
*/
u8 HexToASCIICharLower(u8 u8Char_)
{
  u8 u8ReturnValue = 0xFF;
  
  if( u8Char_ <= 9 )
  {
    u8ReturnValue = u8Char_ + (u8)48;
  }
  
  else if( (u8Char_ >= 10) && (u8Char_ <= 15) )
  {
    u8ReturnValue = u8Char_ + (u8)87;
  }
  
  return(u8ReturnValue);

} /* end HexToASCIICharLower */


/*-----------------------------------------------------------------------------/
Function: NumberToAscii

Description:
Converts a long into an ASCII string.  Maximum of 10 digits + NULL.

Requires:
  - u32Number_ is the number to convert
  - *pu8AsciiString_ points to the destination string location
 
Promises:
  - Null-terminated string of the number is loaded to pu8AsciiString_
  - Returns the number of digits
*/
u8 NumberToAscii(u32 u32Number_, u8* pu8AsciiString_)
{
  bool bFoundDigit = FALSE;
  u8 au8AsciiNumber[11];
  u8 u8CharCount = 0;
  u32 u32Divider = 1000000000;
  u8 u8Temp;
  
  /* Parse out all the digits, start counting after leading zeros */
  for(u8 i = 0; i < 10; i++)
  {
    u8Temp = (u32Number_ / u32Divider);
    
    /* Check for first non-leading zero */
    if(u8Temp != 0)
    {
      bFoundDigit = TRUE;
    }

    /* As long as a non-leading zero has been found, add the ASCII char */
    if(bFoundDigit)
    {
      au8AsciiNumber[u8CharCount] = u8Temp + 0x30;
      u8CharCount++;
    }
    
    /* Update for next iteration */
    u32Number_ %= u32Divider;
    u32Divider /= 10;
  }
  
  /* Handle special case where u32Number == 0 */
  if(!bFoundDigit)
  {
    u8CharCount = 1;
    au8AsciiNumber[0] = '0';
  }
  
  /* Add the null and copy to destination */
  au8AsciiNumber[u8CharCount] = 0;
  strcpy((char *)pu8AsciiString_, (const char*)au8AsciiNumber);
  
  return(u8CharCount);

} /* end NumberToAscii() */


/*-----------------------------------------------------------------------------/
Function: SearchString

Description:
Searches a string for another string.  Finds only an exact match of the string (case sensitive).
Character following matched string must be space, <CR> or <LF>.

Requires:
  - Standard ASCII table is in use
  - pu8TargetString_ points to the start of a NULL, <CR> or <LF> terminated string to search
  - pu8MatchString_ points to the start of a NULL, <CR> or <LF> terminated string to be found in pu8TargetString_
 
Promises:
  - Returns TRUE if the string is found
*/
bool SearchString(u8* pu8TargetString_, u8* pu8MatchString_)
{
  u8* pu8MatchChar = pu8MatchString_;
  u8* pu8TargetChar = pu8TargetString_;
  
  do
  {
    /* Scan for the current character of pu8MatchString_ in pu8TargetString_ */
    while( (*pu8MatchChar != *pu8TargetChar) && (*pu8TargetChar != 0) && 
           (*pu8TargetChar != (u8)0x0A) && (*pu8TargetChar != (u8)0x0D) )
    {
      pu8TargetChar++;
    }
    
    /* Exit if we're at the end of the target string */
    if( (*pu8TargetChar == 0) || 
        (*pu8TargetChar == (u8)0x0A) || (*pu8TargetChar == (u8)0x0D) )
    {
      return(FALSE);
    }
    
    /* If we found a match, look for next char */
    while ( *pu8MatchChar == *pu8TargetChar )
    {
      pu8MatchChar++;
      pu8TargetChar++;
      
      /* At the end of the match string? */
      if( (*pu8MatchChar == 0) || (*pu8MatchChar == (u8)0x0A) || (*pu8MatchChar == (u8)0x0D) )
      {
        /* Check if the next character in pu8TargetChar is space, <CR>, <LF> or ':' */
        if( (*pu8TargetChar == ' ') ||
            (*pu8TargetChar == (u8)0x0D) ||
            (*pu8TargetChar == (u8)0x0A) ||
            (*pu8TargetChar == ':')  )
        {
          return(TRUE);
        }
      }
    }

    /* At the end of the target string? */
    if( (*pu8TargetChar == 0) || (*pu8TargetChar == (u8)0x0A) || (*pu8TargetChar == (u8)0x0D) )
    {
      return(FALSE);
    }
    
    /* If here, match was not found (yet), but as long as there are more characters in pu8TargetChar,
    then we can scan again. */
    
    /* Reset match pointer back to the start of its string */
    pu8MatchChar = pu8MatchString_;
  } while ( (*pu8TargetChar != 0) && 
            (*pu8TargetChar != (u8)0x0A) && (*pu8TargetChar != (u8)0x0D) );
  
  /* If we get here, no match was found */
  return(FALSE);

} /* end SearchString */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
