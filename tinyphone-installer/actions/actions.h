/**
 * Function prototypes for external "C" interfaces into the DLL.
 *
 * This project builds a "hybrid" DLL that will work either from
 * a MSI Custom Action environment or from an external C program.
 * The former routes through "C" interface functions defined in 
 * CustomAction.def.  The latter uses the interfaces defined here.
 *
 * This header is suitable for inclusion by a project wanting to
 * call these methods.  Note that _DEVMSI_EXPORTS should not be
 * defined for the accessing application source code.
 */
#pragma once

#ifdef _DEVMSI_EXPORTS
#  define DEVMSI_API __declspec(dllexport)
#else
#  define DEVMSI_API __declspec(dllimport)
#endif

 /**
 * PostInstall 
 *
 * This method run neccessary post install
 *
 * @param argc  The count of valid arguments in argv.
 * @param argv  An array of string arguments for the function.
 * @return Returns an HRESULT indicating success or failure.
 */
HRESULT DEVMSI_API DoPostInstall(int argc, LPWSTR* argv);


/**
* PreInstall
*
* This method run neccessary post install
*
* @param argc  The count of valid arguments in argv.
* @param argv  An array of string arguments for the function.
* @return Returns an HRESULT indicating success or failure.
*/
HRESULT DEVMSI_API DoPreInstall(int argc, LPWSTR* argv);


/**
 *  Standardized function prototype for DevMsi.
 *
 *  Functions in DevMsi can be called through the MSI Custom
 *  Action DLL or through an external C program.  Both
 *  methods expect to wrap things into this function prototype.
 *
 *  As a result, all functions defined in this header should
 *  conform to this function prototype.
 */
typedef HRESULT DEVMSI_API (*CUSTOM_ACTION_ARGC_ARGV)(
    int argc, LPWSTR* argv
    );
