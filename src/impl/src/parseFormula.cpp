#include <cstdio>
#include <limits>
#include <sstream>
#include <stack>

#include "parseException.hpp"
#include "parseFormula.hpp"

//! Infix bracket state

/**
 * Infix bracket completion state. The order must be preserved.
 */
enum InfixState
{
    UNARY, ///< Unary operator set
    BLANK, ///< New bracket openned
    FIRST_OPERAND, ///< First operand set
    BINARY, ///< Binary operator set
    LAST_OPERAND ///< Last operand set
};

Formula* parsePrefix(istream& input)
{
    unsigned position = 1;
    bool run = true;
    stack<Composite*> operatorStack;
    Formula* temporary = NULL;

    while (run)
    {
        char buffer = input.get();
        switch (buffer)
        {
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
                if (!operatorStack.empty())
                {
                    // Set this as a operator stack top's operand
                    temporary = new Trivial(buffer);

                    // While all operator stack top's operands have been set
                    while (!operatorStack.empty() &&
                           operatorStack.top()->setFirst(temporary))
                    {
                        temporary = operatorStack.top();
                        operatorStack.pop();
                    }
                } else if (position == 1)
                {
                    // Trivial formula case
                    temporary = new Trivial(buffer);
                } else
                {
                    // No more operands are needed
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    delete temporary;
                    throw RedundantElementException(buffer, position);
                }
                break;
            case '-':
                if (!operatorStack.empty())
                {
                    // Push this to the operator stack
                    operatorStack.push(new Unary(buffer));
                } else if (position == 1)
                {
                    // Initial operator case
                    operatorStack.push(new Unary(buffer));
                } else
                {
                    // No more operands are needed
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    delete temporary;
                    throw RedundantElementException(buffer, position);
                }
                break;
            case '.':
            case '+':
            case '>':
            case '=':
                if (!operatorStack.empty())
                {
                    // Push this to the operator stack
                    operatorStack.push(new Binary(buffer));
                } else if (position == 1)
                {
                    // Initial operator case
                    operatorStack.push(new Binary(buffer));
                } else
                {
                    // No more operands are needed
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    delete temporary;
                    throw RedundantElementException(buffer, position);
                }
                break;
            case ' ':
            case '\t':
                // Legal whitespace skipping
                position--;
                break;
            case '\n':
                // Formula proper ending
                run = false;
                break;
            case EOF:
                if (position == 1)
                {
                    // End of formula sequence
                    run = false;
                } else
                {
                    // Unexpected end of stream
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    throw UnexpectedEOFException();
                }
                break;
            default:
                // Illegal character used
                input.ignore(numeric_limits<streamsize>::max(), '\n');
                while (!operatorStack.empty())
                {
                    delete operatorStack.top();
                    operatorStack.pop();
                }
                throw IllegalCharacterException(buffer, position);
                break;
        }
        position++;
    }
    if (operatorStack.empty())
    {
        // Formula is complete
        return temporary;
    } else
    {
        // Formula is incomplete
        while (!operatorStack.empty())
        {
            delete operatorStack.top();
            operatorStack.pop();
        }
        throw IncompleteFormulaException();
    }
}

Formula* parseInfix(istream& input)
{
    unsigned position = 1;
    bool run = true;
    stack<Formula*> formulaStack;
    stack<Composite*> operatorStack;
    stack<int> stateStack;
    Formula* temporary;

    while (run)
    {
        char buffer = input.get();
        switch (buffer)
        {
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
                if (stateStack.empty())
                {
                    if (position == 1)
                    {
                        // Trivial formula case
                        formulaStack.push(new Trivial(buffer));
                    } else
                    {
                        // Unexpected element position
                        input.ignore(numeric_limits<streamsize>::max(), '\n');
                        while (!formulaStack.empty())
                        {
                            delete formulaStack.top();
                            formulaStack.pop();
                        }
                        while (!operatorStack.empty())
                        {
                            delete operatorStack.top();
                            operatorStack.pop();
                        }
                        throw UnexpectedElementException(buffer, position);
                    }
                } else if (stateStack.top() == BLANK ||
                           stateStack.top() == BINARY)
                {
                    // The first or the last operand case
                    formulaStack.push(new Trivial(buffer));
                    stateStack.top()++;
                } else if (stateStack.top() == UNARY)
                {
                    // Unary operator operand
                    temporary = new Trivial(buffer);

                    // Close recent unary operators
                    do
                    {
                        operatorStack.top()->setFirst(temporary);
                        temporary = operatorStack.top();
                        operatorStack.pop();
                        stateStack.pop();
                    } while (!stateStack.empty() &&
                             stateStack.top() == UNARY);

                    // Push this to the formula stack
                    formulaStack.push(temporary);
                    if (stateStack.size())
                    {
                        stateStack.top()++;
                    }
                } else
                {
                    // Unexpected element position
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    throw UnexpectedElementException(buffer, position);
                }
                break;
            case '-':
                if (stateStack.empty() && position != 1)
                {
                    // No more elements are needed
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    throw RedundantElementException(buffer, position);
                } else if ((stateStack.empty() && position == 1) ||
                           stateStack.top() == BLANK ||
                           stateStack.top() == BINARY ||
                           stateStack.top() == UNARY)
                {
                    // Push this to the operator stack
                    stateStack.push(UNARY);
                    operatorStack.push(new Unary(buffer));
                } else
                {
                    // Unexpected element position
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    throw UnexpectedElementException(buffer, position);
                }
                break;
            case '.':
            case '+':
            case '>':
            case '=':
                if (stateStack.empty())
                {
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (position == 1)
                    {
                        // Unexpected element position
                        while (!formulaStack.empty())
                        {
                            delete formulaStack.top();
                            formulaStack.pop();
                        }
                        while (!operatorStack.empty())
                        {
                            delete operatorStack.top();
                            operatorStack.pop();
                        }
                        throw UnexpectedElementException(buffer, position);
                    } else
                    {
                        // No more elements are needed
                        while (!formulaStack.empty())
                        {
                            delete formulaStack.top();
                            formulaStack.pop();
                        }
                        while (!operatorStack.empty())
                        {
                            delete operatorStack.top();
                            operatorStack.pop();
                        }
                        throw RedundantElementException(buffer, position);
                    }
                } else if (stateStack.top() == FIRST_OPERAND)
                {
                    // Between operands case
                    stateStack.top() = BINARY;
                    operatorStack.push(new Binary(buffer));
                } else
                {
                    // Unexpected element position
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    throw UnexpectedElementException(buffer, position);
                }
                break;
            case '(':
                if (stateStack.empty())
                {
                    if (position == 1)
                    {
                        // Initial openning bracket case
                        stateStack.push(BLANK);
                    } else
                    {
                        // No more elements are needed
                        input.ignore(numeric_limits<streamsize>::max(), '\n');
                        while (!formulaStack.empty())
                        {
                            delete formulaStack.top();
                            formulaStack.pop();
                        }
                        while (!operatorStack.empty())
                        {
                            delete operatorStack.top();
                            operatorStack.pop();
                        }
                        throw RedundantElementException(buffer, position);
                    }
                } else if (stateStack.top() == BLANK ||
                           stateStack.top() == BINARY ||
                           stateStack.top() == UNARY)
                {
                    // Tree level openning
                    stateStack.push(BLANK);
                } else
                {
                    // Unexpected element position
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    throw UnexpectedElementException(buffer, position);
                }
                break;
            case ')':
                if (stateStack.empty())
                {
                    // Too much closing brackets case
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    throw RedundantElementException(buffer, position);
                } else if (stateStack.top() == LAST_OPERAND)
                {
                    // Tree level closing
                    stateStack.pop();
                    do
                    {
                        temporary = formulaStack.top();
                        formulaStack.pop();
                    } while (!operatorStack.top()->setLast(temporary));
                    formulaStack.push(operatorStack.top());
                    operatorStack.pop();

                    // Close recent unary operators
                    while (stateStack.size() && stateStack.top() == UNARY)
                    {
                        operatorStack.top()->setFirst(formulaStack.top());
                        formulaStack.pop();
                        formulaStack.push(operatorStack.top());
                        operatorStack.pop();
                        stateStack.pop();
                    }

                    if (stateStack.size())
                    {
                        stateStack.top()++;
                    }
                } else
                {
                    // Unexpected element position
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    throw UnexpectedElementException(buffer, position);
                }
                break;
            case ' ':
            case '\t':
                // Legal whitespace skipping
                position--;
                break;
            case '\n':
                // Proper formula ending
                run = false;
                break;
            case EOF:
                if (position == 1)
                {
                    // End of formula sequence
                    run = false;
                } else
                {
                    // Unexpected end of stream
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    while (!operatorStack.empty())
                    {
                        delete operatorStack.top();
                        operatorStack.pop();
                    }
                    throw UnexpectedEOFException();
                }
                break;
            default:
                // Illegal character used
                input.ignore(numeric_limits<streamsize>::max(), '\n');
                while (!formulaStack.empty())
                {
                    delete formulaStack.top();
                    formulaStack.pop();
                }
                while (!operatorStack.empty())
                {
                    delete operatorStack.top();
                    operatorStack.pop();
                }
                throw IllegalCharacterException(buffer, position);
                break;
        }
        position++;
    }
    if (stateStack.empty())
    {
        if (position == 2)
        {
            // Empty line was parsed
            return NULL;
        } else
        {
            // Formula is complete
            return formulaStack.top();
        }
    } else
    {
        // Formula is incomplete
        while (!formulaStack.empty())
        {
            delete formulaStack.top();
            formulaStack.pop();
        }
        while (!operatorStack.empty())
        {
            delete operatorStack.top();
            operatorStack.pop();
        }
        throw IncompleteFormulaException();
    }
}

Formula* parsePostfix(istream& input)
{
    unsigned position = 1;
    bool run = true;
    stack<Formula*> formulaStack;
    Composite* temporary;

    while (run)
    {
        char buffer = input.get();
        switch (buffer)
        {
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
                // Push this to the formula stack
                formulaStack.push(new Trivial(buffer));
                break;
            case '-':
                if (!formulaStack.empty())
                {
                    // Stack operand is available
                    temporary = new Unary(buffer);
                    temporary->setLast(formulaStack.top());
                    formulaStack.pop();
                    formulaStack.push(temporary);
                } else
                {
                    // Not enough stack operands
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    throw RedundantElementException(buffer, position);
                }
                break;
            case '.':
            case '+':
            case '>':
            case '=':
                if (formulaStack.size() > 1)
                {
                    // Stack operands are available
                    temporary = new Binary(buffer);
                    temporary->setLast(formulaStack.top());
                    formulaStack.pop();
                    temporary->setLast(formulaStack.top());
                    formulaStack.pop();
                    formulaStack.push(temporary);
                } else
                {
                    // Not enough stack operands
                    input.ignore(numeric_limits<streamsize>::max(), '\n');
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    throw RedundantElementException(buffer, position);
                }
                break;
            case ' ':
            case '\t':
                // Legal whitespace skipping
                position--;
                break;
            case '\n':
                // Proper formula ending
                run = false;
                break;
            case EOF:
                if (position == 1)
                {
                    // End of formula sequence
                    run = false;
                } else
                {
                    // Unexpected end of stream
                    while (!formulaStack.empty())
                    {
                        delete formulaStack.top();
                        formulaStack.pop();
                    }
                    throw UnexpectedEOFException();
                }
                break;
            default:
                // Illegal character used
                input.ignore(numeric_limits<streamsize>::max(), '\n');
                while (!formulaStack.empty())
                {
                    delete formulaStack.top();
                    formulaStack.pop();
                }
                throw IllegalCharacterException(buffer, position);
                break;
        }
        position++;
    }
    if (position == 2)
    {
        // Empty line was parsed
        return NULL;
    } else if (formulaStack.size() == 1)
    {
        // Formula is complete
        return formulaStack.top();
    } else
    {
        // Formula is incomplete
        while (!formulaStack.empty())
        {
            delete formulaStack.top();
            formulaStack.pop();
        }
        throw IncompleteFormulaException();
    }
}
