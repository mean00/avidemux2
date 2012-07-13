/** \defgroup globalFunctions Global Functions
 * \{
 */

/** \brief Executes the specified command with the specified arguments.
 */
void execute(String command, ...);


/** \brief Adds the contents of the specified file to the source script at the point where the function is called.
 */
void include(String scriptPath);

/** \brief Writes the passed arguments to the standard output and the Debug Output widget of the script debugger.
 * It is intended to be used for simple debugging purposes.
 */
void print(...);

/** \}
 */
