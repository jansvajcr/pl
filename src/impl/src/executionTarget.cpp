#include <iostream>
#include <list>
#include <stdlib.h>

#include "executionTarget.hpp"
#include "formula.hpp"
#include "parseException.hpp"
#include "proofSystem.hpp"

ExecutionTarget::~ExecutionTarget()
{
}

int DefaultTarget::execute(Configuration& config) const
{
    int exit = EXIT_SUCCESS;

    while (true)
    {
        try
        {
            // Formula parsing
            Formula* formula = config.parseFormula();
            if (formula == NULL)
            {
                break;
            }

            // Formula printing
            if (config.getEcho())
            {
                cout << config.printFormula(formula) << endl;
            }
            delete formula;
        } catch (ParseException& exception)
        {
            if (config.getEcho())
            {
                cerr << exception.getMessage() << endl;
            }
            exit = EXIT_FAILURE;
            if (config.getStrict())
            {
                break;
            }
        }
    }
    return exit;
}

int AxiomChecker::execute(Configuration& config) const
{
    int exit = EXIT_SUCCESS;

    while (true)
    {
        try
        {
            // Formula parsing
            Formula* formula = config.parseFormula();
            if (formula == NULL)
            {
                break;
            }

            // Axiom checking
            unsigned type = config.getSystem()->isAxiom(formula);
            delete formula;
            if (type > 0)
            {
                if (config.getEcho())
                {
                    cout << "Axiom of type " << type << "." << endl;
                }
            } else
            {
                if (config.getEcho())
                {
                    cout << "Not an axiom." << endl;
                }
                exit = EXIT_FAILURE;
                if (config.getStrict())
                {
                    break;
                }
            }
        } catch (ParseException& exception)
        {
            if (config.getEcho())
            {
                cerr << exception.getMessage() << endl;
            }
            exit = EXIT_FAILURE;
            if (config.getStrict())
            {
                break;
            }
        }
    }
    return exit;
}

ProofHandler::ProofHandler(unsigned premises,
                           ProofTarget target)
: premises(premises), target(target)
{
}

int ProofHandler::execute(Configuration& config) const
{
    int exit = EXIT_SUCCESS;
    unsigned premisesLeft = premises;
    list<Formula*> theory;
    vector<ProofMember*> proof;

    while (true)
    {
        try
        {
            // Formula parsing
            Formula* formula = config.parseFormula();
            if (formula == NULL)
            {
                break;
            }
            if (premisesLeft > 0)
            {
                theory.push_back(formula);
                premisesLeft--;
                continue;
            }

            // Axiom checking
            unsigned type = config.getSystem()->isAxiom(formula);
            if (type > 0)
            {
                if (target == VERIFY && config.getEcho())
                {
                    cout << "Axiom of type " << type << "." << endl;
                }
                proof.push_back(new ProofMember(formula));
                continue;
            }

            // Theory member checking
            type = 1;
            for (Formula* premise: theory)
            {
                if (formula->equals(premise))
                {
                    break;
                }
                type++;
            }
            if (type <= theory.size())
            {
                if (target == VERIFY && config.getEcho())
                {
                    cout << "Premise of type " << type << "." << endl;
                }
                proof.push_back(new ProofMember(formula));
                continue;
            }

            // Deduction checking
            list<unsigned> indexes = config.getSystem()->isDeducible(formula, proof);
            if (!indexes.empty())
            {
                if (target == VERIFY && config.getEcho())
                {
                    cout << "Deducible using formulas ";
                    for (unsigned index: indexes)
                    {
                        cout << index << " ";
                    }
                    cout << "as witnesses." << endl;
                }
                list<ProofMember*> witnesses;
                for (unsigned index: indexes)
                {
                    witnesses.push_back(proof[index - 1]);
                }
                proof.push_back(new ProofMember(formula, witnesses));
                continue;
            }

            // Formula is not deducible
            if (config.getEcho())
            {
                switch (target)
                {
                    case VERIFY:
                        cout << "Formula not deducible." << endl;
                        break;
                    case OPTIMIZE:
                        cerr << "Invalid proof given." << endl;
                        break;
                }
            }
            exit = EXIT_FAILURE;
            delete formula;
            break;
        } catch (ParseException& exception)
        {
            if (config.getEcho())
            {
                switch (target)
                {
                    case VERIFY:
                        cerr << exception.getMessage() << endl;
                        break;
                    case OPTIMIZE:
                        cerr << "Invalid formula " << proof.size() + 1 << "." << endl;
                        break;
                }
            }
            exit = EXIT_FAILURE;
            break;
        }
    }

    // Proof optimization
    if (exit == EXIT_SUCCESS && target == OPTIMIZE)
    {
        list<ProofMember*> queue;
        unsigned preserved = 0;

        queue.push_back(proof.back());
        while (!queue.empty())
        {
            for (ProofMember* witness: queue.front()->getWitnesses())
            {
                queue.push_back(witness);
            }
            queue.front()->setPreserve(true);
            queue.pop_front();
            preserved++;
        }
        if (preserved == proof.size())
        {
            if (config.getEcho())
            {
                cerr << "Proof already optimal." << endl;
            }
            exit = EXIT_FAILURE;
        } else
        {
            if (config.getEcho())
            {
                for (ProofMember* member: proof)
                {
                    if (member->getPreserve())
                    {
                        cout << config.printFormula(member->getFormula()) << endl;
                    }
                }
            }
        }
    }
    
    // Cleanup
    for (Formula* formula: theory)
    {
        delete formula;
    }
    for (ProofMember* member: proof)
    {
        delete member;
    }
    return exit;
}
