/**
 * Log2Log
 *  Formats
 *   Standard Converter
 *
 * This class shows the persistent structure of all standard converter methods.
 * All of the functions should be overloaded.
 */

#include "stdconverter.h"

/**
 * Constructor
 */
StdConverter::StdConverter()
{
}

/**
 * Destructor
 */
StdConverter::~StdConverter()
{
}

/**
 * Load a Chat Log
 */
QVariant StdConverter::load()
{
}

/**
 * Delete All Work
 */
void StdConverter::unset()
{
    delete final;
}

/**
 * Process "From" Request
 */
StdFormat StdConverter::from(QHash<QString, QVariant> data)
{
}

/**
 * Process "To" Request
 */
void StdConverter::to()
{
}
