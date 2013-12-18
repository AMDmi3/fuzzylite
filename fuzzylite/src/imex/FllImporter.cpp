/*   Copyright 2013 Juan Rada-Vilela

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

/* 
 * File:   FllImporter.cpp
 * Author: jcrada
 *
 * Created on 18 December 2013, 11:42 AM
 */

#include "fl/imex/FllImporter.h"

#include "fl/Headers.h"

namespace fl {

    FllImporter::FllImporter() {
    }

    FllImporter::~FllImporter() {

    }

    std::string FllImporter::name() const {
        return "FllImporter";
    }

    Engine* FllImporter::fromString(const std::string& fll) const {
        Engine* engine = new Engine;

        std::string tag;
        std::ostringstream block;
        bool processPending = false;
        std::istringstream fclReader(fll);
        std::string line;
        int lineNumber = 0;
        try {
            while (std::getline(fclReader, line)) {
                ++lineNumber;
                line = clean(line);
                if (line.empty()) continue;
                std::size_t colon = line.find_first_of(':');
                if (colon == std::string::npos) {
                    throw fl::Exception("[import error] expected a colon in line #" +
                            Op::str(lineNumber) + ": " + line, FL_AT);
                }
                std::string key = Op::trim(line.substr(0, colon));
                std::string value = Op::trim(line.substr(colon + 1));
                if ("Engine" == key) {
                    engine->setName(value);
                    continue;
                } else {
                    processPending = (key == "InputVariable" or key == "OutputVariable"
                            or key == "RuleBlock");
                }
                if (processPending) {
                    process(tag, block.str(), engine);
                    //TODO: clear vs str("");?
                    block.clear();
                    block.str("");
                    processPending = false;
                    tag = key;
                }
                block << key << ":" << value << "\n";
            }
            process(tag, block.str(), engine);
        } catch (fl::Exception& ex) {
            delete engine;
            throw ex;
        }
        return engine;
    }

    void FllImporter::process(const std::string& tag, const std::string& block, Engine* engine) const {
        if (tag.empty()) return;
        if ("InputVariable" == tag) {
            processInputVariable(block, engine);
        } else if ("OutputVariable" == tag) {
            processOutputVariable(block, engine);
        } else if ("RuleBlock" == tag) {
            processRuleBlock(block, engine);
        } else {
            throw fl::Exception("[import error] block tag <" + tag + "> not recognized", FL_AT);
        }
    }

    void FllImporter::processInputVariable(const std::string& block, Engine* engine) const {
        std::istringstream reader(block);
        std::string line;
        InputVariable* inputVariable = new InputVariable;
        engine->addInputVariable(inputVariable);
        while (std::getline(reader, line)) {
            std::pair<std::string, std::string> keyValue = parseKeyValue(line, ':');
            if ("InputVariable" == keyValue.first) {
                inputVariable->setName(keyValue.second);
            } else if ("enabled" == keyValue.first) {
                inputVariable->setEnabled("true" == keyValue.second);
            } else if ("range" == keyValue.first) {
                std::pair<scalar, scalar> range = parseRange(keyValue.second);
                inputVariable->setRange(range.first, range.second);
            } else if ("Term" == keyValue.first) {
                inputVariable->addTerm(parseTerm(keyValue.second, engine));
            } else {
                throw fl::Exception("[import error] key <" + keyValue.first + "> not "
                        "recognized in pair <" + keyValue.first + ":" + keyValue.second + ">", FL_AT);
            }
        }
    }

    void FllImporter::processOutputVariable(const std::string& block, Engine* engine) const {
        std::istringstream reader(block);
        std::string line;
        OutputVariable* outputVariable = new OutputVariable;
        engine->addOutputVariable(outputVariable);
        while (std::getline(reader, line)) {
            std::pair<std::string, std::string> keyValue = parseKeyValue(line, ':');
            if ("OutputVariable" == keyValue.first) {
                outputVariable->setName(keyValue.second);
            } else if ("enabled" == keyValue.first) {
                outputVariable->setEnabled("true" == keyValue.second);
            } else if ("range" == keyValue.first) {
                std::pair<scalar, scalar> range = parseRange(keyValue.second);
                outputVariable->setRange(range.first, range.second);
            } else if ("default" == keyValue.first) {
                outputVariable->setDefaultValue(Op::toScalar(keyValue.second));
            } else if ("lock-valid" == keyValue.first) {
                outputVariable->setLockValidOutput("true" == keyValue.second);
            } else if ("lock-range" == keyValue.first) {
                outputVariable->setLockOutputRange("true" == keyValue.second);
            } else if ("defuzzifier" == keyValue.first) {
                outputVariable->setDefuzzifier(parseDefuzzifier(keyValue.second));
            } else if ("accumulation" == keyValue.first) {
                outputVariable->output()->setAccumulation(parseSNorm(keyValue.second));
            } else if ("Term" == keyValue.first) {
                outputVariable->addTerm(parseTerm(keyValue.second, engine));
            } else {
                throw fl::Exception("[import error] key <" + keyValue.first + "> not "
                        "recognized in pair <" + keyValue.first + ":" + keyValue.second + ">", FL_AT);
            }
        }
    }

    void FllImporter::processRuleBlock(const std::string& block, Engine* engine) const {
        std::istringstream reader(block);
        std::string line;
        RuleBlock* ruleBlock = new RuleBlock;
        engine->addRuleBlock(ruleBlock);
        while (std::getline(reader, line)) {
            std::pair<std::string, std::string> keyValue = parseKeyValue(line, ':');
            if ("RuleBlock" == keyValue.first) {
                ruleBlock->setName(keyValue.second);
            } else if ("enabled" == keyValue.first) {
                ruleBlock->setEnabled("true" == keyValue.second);
            } else if ("conjunction" == keyValue.first) {
                ruleBlock->setConjunction(parseTNorm(keyValue.second));
            } else if ("disjunction" == keyValue.first) {
                ruleBlock->setDisjunction(parseSNorm(keyValue.second));
            } else if ("activation" == keyValue.first) {
                ruleBlock->setActivation(parseTNorm(keyValue.second));
            } else if ("Rule" == keyValue.first) {
                ruleBlock->addRule(fl::Rule::parse(keyValue.second, engine));
            } else {
                throw fl::Exception("[import error] key <" + keyValue.first + "> not "
                        "recognized in pair <" + keyValue.first + ":" + keyValue.second + ">", FL_AT);
            }
        }
    }

    Term* FllImporter::parseTerm(const std::string& text, Engine* engine) const {
        std::istringstream tokenizer(text);
        std::string token;
        std::vector<std::string> tokens;
        while (tokenizer >> token) {
            tokens.push_back(token);
        }
        //MEDIUM = Triangle 0.500 1.000 1.500

        if (tokens.size() < 3) {
            throw fl::Exception("[syntax error] expected a term in format <name = class parameters>, "
                    "but found <" + text + ">", FL_AT);
        }
        Term* term = FactoryManager::instance()->term()->createInstance(tokens.at(2));
        term->setName(Op::makeValidId(tokens.at(0)));
        std::ostringstream parameters;
        for (std::size_t i = 3; i < tokens.size(); ++i) {
            parameters << tokens.at(i);
            if (i + 1 < tokens.size()) parameters << " ";
        }
        term->configure(parameters.str());
        //special cases:
        Linear* linear = NULL;
        Function* function = NULL;
        if ((linear = dynamic_cast<Linear*> (term))) {
            linear->inputVariables = std::vector<const InputVariable*>
                    (engine->inputVariables().begin(),
                    engine->inputVariables().end());
        } else if ((function = dynamic_cast<Function*> (term))) {
            function->setEngine(engine);
            //builtin functions are loaded from TermFactory calling Function::create
            function->load();
        }
        return term;
    }

    TNorm* FllImporter::parseTNorm(const std::string& name) const {
        if (name.empty() or "none" == name) return NULL;
        return FactoryManager::instance()->tnorm()->createInstance(name);
    }

    SNorm* FllImporter::parseSNorm(const std::string& name) const {
        if (name.empty() or "none" == name) return NULL;
        return FactoryManager::instance()->snorm()->createInstance(name);
    }

    Defuzzifier* FllImporter::parseDefuzzifier(const std::string& text) const {
        if (text.empty() or "none" == text) return NULL;
        std::vector<std::string> parameters = Op::split(text, " ");
        std::string name = parameters.at(0);
        Defuzzifier* defuzzifier = FactoryManager::instance()->defuzzifier()->createInstance(name);
        if (parameters.size() > 1) {
            IntegralDefuzzifier* integralDefuzzifier =
                    dynamic_cast<IntegralDefuzzifier*> (defuzzifier);
            if (integralDefuzzifier) {
                integralDefuzzifier->setResolution(Op::toScalar(parameters.at(1)));
            }
        }
        return defuzzifier;
    }

    std::pair<scalar, scalar> FllImporter::parseRange(const std::string& text) const {
        std::pair<std::string, std::string> range = parseKeyValue(text, ' ');
        return std::pair<scalar, scalar>(Op::toScalar(range.first), Op::toScalar(range.second));
    }

    std::pair<std::string, std::string> FllImporter::parseKeyValue(const std::string& text,
            char separator) const {
        std::size_t half = text.find_first_of(separator);
        if (half == std::string::npos) {
            std::ostringstream ex;
            ex << "[syntax error] expected pair in the form "
                    "<key" << separator << "value>, but found <" << text << ">";
            throw fl::Exception(ex.str(), FL_AT);
        }
        std::pair<std::string, std::string> result;
        result.first = text.substr(0, half);
        result.second = text.substr(half + 1);
        return result;
    }

    std::string FllImporter::clean(const std::string& line) const {
        if (line.empty()) return line;
        if (line.size() == 1) return std::isspace(line.at(0)) ? "" : line;
        int start = 0, end = line.size() - 1;
        while (start <= end and std::isspace(line.at(start))) {
            ++start;
        }
        int sharp = start;
        while (sharp <= end) {
            if (line.at(sharp) == '#') {
                end = sharp - 1; //prevent negative if sharp = 0
                break;
            }
            ++sharp;
        }
        //FL_LOG("s:" << start << " #:" << sharp << " e:" << end);
        while (end >= start and (line.at(end) == '#' or std::isspace(line.at(end)))) {
            --end;
        }

        int length = end - start + 1;
        return line.substr(start, length);
    }

    int FllImporter::main() {
        FllImporter importer;
        FllExporter exporter;
        Engine* engine = Console::mamdani();
        std::string fll = exporter.toString(engine);

        Engine* back = importer.fromString(fll);
        std::string fllback = exporter.toString(back);
        FL_LOG(fll);
        if (fll == fllback) {
            FL_LOG("FLL SUCCESSFUL")
        } else {
            FL_LOG("FLL FAILED");
        }

        return 0;
    }


}