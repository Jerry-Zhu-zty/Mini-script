#pragma once
#include "script.h"
bool ScriptEngine::IsOper(char c)
{
    string str;
    str = c;
    for (auto a : g_sOperationList)
    {
        if (a == str)
            return true;
    }
    return false;
}
bool ScriptEngine::IsNumber(const string &s)
{
    bool bHasDot = false;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == '.')
        {
            if (bHasDot)
                return false;
            bHasDot = true;
        }
        else if (!isdigit(s[i]))
        {
            return false;
        }
    }
    return true;
}
#define LOGVAR(var) cout << var->sName << ": " << var->sValue << " (Type: " << var->eVarType << ")" << endl
VARSAVETYPE *ScriptEngine::CreateVarData(const string &value)
{
    // TODO:throw error if value is an undefined variable
    VARSAVETYPE *p = new VARSAVETYPE;
    // if (!IsNumber(value))
    //{
    if (value.size() >= 2 &&
        value[0] == '"' && value.back() == '"')
    {
        // remove quota
        string strValue = value.substr(1, value.size() - 2);
        p->sName = "STR";
        p->eVarType = STR;
        p->sValue = strValue;
    }
    //}
    else
    {
        p->sName = "VAR";
        p->eVarType = NUM;
        p->sValue = value;
    }
    p->nScoop = m_nScoopCount;
    // LOGVAR(p);
    return p;
}
bool ScriptEngine::CompareValues(const string &op, const string &left, const string &right)
{
    if (IsNumber(left) && IsNumber(right))
    {
        double l = stod(left);
        double r = stod(right);
        if (op == "==")
            return fabs(l - r) < 1e-9;
        if (op == "!=")
            return fabs(l - r) >= 1e-9;
        if (op == "<")
            return l < r;
        if (op == ">")
            return l > r;
        if (op == "<=")
            return l <= r;
        if (op == ">=")
            return l >= r;
    }
    else
    {
        if (op == "==")
            return left == right;
        if (op == "!=")
            return left != right;
    }
    return false;
}
CHARTYPE ScriptEngine::CheckCharType(char c)
{
    if (isdigit(c) || c == '.')
        return DIGIT;
    if (isalpha(c) || c == '_' || c == '\"')
        return ALPHA;
    if (c == ';')
        return SEPARATE;
    if (c == '\n')
    {
        return ENTER;
    }
    if (isspace(c))
    {
        return SPACE;
    }
    if (IsOper(c))
        return OPER;
    // if (c == '"')
    //    return VOID;
    return VOID;
}

vector<string> ScriptEngine::Slice(string str)
{
    string sCode = str;
    string sCurrent = "";
    CHARTYPE eCurCharType;
    vector<string> vsCodeData;
    bool bInStr = false;

    for (size_t i = 0; i < sCode.length(); i++)
    {
        if (sCode[i] == '"')
        {
            if (!bInStr)
            {
                if (!sCurrent.empty())
                {
                    vsCodeData.push_back(sCurrent);
                    sCurrent.clear();
                }
                bInStr = true;
                sCurrent = "\"";
            }
            else
            {
                sCurrent += "\"";
                vsCodeData.push_back(sCurrent);
                sCurrent.clear();
                bInStr = false;
            }
            continue;
        }

        if (bInStr)
        {
            sCurrent += sCode[i];
            continue;
        }

        sCurrent = sCurrent + sCode[i];
        eCurCharType = CheckCharType(sCode[i]);

        if (CheckCharType(sCurrent[0]) == SPACE)
        {
            sCurrent.clear();
            continue;
        }

        if (i < sCode.length())
        {
            if (CheckCharType(sCode[i + 1]) != eCurCharType)
            {
                vsCodeData.push_back(sCurrent);
                sCurrent.clear();
            }
        }
        else
        {
            vsCodeData.push_back(sCurrent);
            sCurrent.clear();
        }
    }

    if (!sCurrent.empty())
    {
        vsCodeData.push_back(sCurrent);
    }

    return vsCodeData;
}

int ScriptEngine::BuildTree(NODE *pNode, vector<string> vsCodeData, int nPos)
{
    NODE *pAdd = NULL;
    bool bTemp = true;
    for (size_t i = nPos; i < vsCodeData.size(); i++)
    {
        if (vsCodeData[i] == "(")
        {
            pAdd = new NODE;
            pNode->vpChild.push_back(pAdd);
            i = BuildTree(pAdd, vsCodeData, i + 1);
        }
        else if (vsCodeData[i] == ")")
        {
            return i;
        }
        else if (CheckCharType(vsCodeData[i][0]) == ALPHA || CheckCharType(vsCodeData[i][0]) == DIGIT || CheckCharType(vsCodeData[i][0]) == OPER)
        {
            // assert(vsCodeData[i][0]=='(');
            if (bTemp == true)
            {
                pNode->sValue = vsCodeData[i];
                bTemp = false;
            }
            else
            {
                pAdd = new NODE;
                pAdd->sValue = vsCodeData[i];
                pNode->vpChild.push_back(pAdd);
                // assert(pNode->sValue == "array_new");
            }
        }
    }
    return vsCodeData.size();
}
CTRLTYPE ScriptEngine::CheckCtrlType(string str)
{
    if (str == "for")
        return FOR;
    if (str == "while")
        return WHILE;
    else if (str == "if")
        return IF;
    else if (str == "let")
        return LET;
    else if (str == "exp")
        return EXP;
    else if (str == "write")
        return WRITE;
    else if (str == "program")
        return PROGRAM;
    else if (str == "function")
        return FUNCTION;
    else if (str == "array_set")
        return ARRAYSET;
    else if (str == "array_get")
        return ARRAYGET;
    else if (str == "array_new")
        return ARRAYNEW;
    else if (str == "convert")
        return CONVERT;
    else
        return UNKNOWN;
}

void ScriptEngine::BeginScoop()
{
    m_nScoopCount++;
}
void ScriptEngine::EndScoop()
{
    auto newEnd = remove_if(m_vVar.begin(), m_vVar.end(),
                            [&](const VARSAVETYPE &var)
                            { return var.nScoop == m_nScoopCount; });
    m_vVar.erase(newEnd, m_vVar.end());
    auto newEndArr = remove_if(m_vArray.begin(), m_vArray.end(),
                               [&](const ARRAYSAVETYPE &arr)
                               { return arr.nScoop == m_nScoopCount; });
    m_vArray.erase(newEndArr, m_vArray.end());
    m_nScoopCount--;
}
void ScriptEngine::StatementProgram(NODE *pNode)
{
    BeginScoop();
    // for_each(pNode->vpChild.begin(), pNode->vpChild.end(), [](NODE *pNode)
    //          { Visit(pNode); });
    for (auto a : pNode->vpChild)
    {
        Visit(a);
    }
    EndScoop();
}

void ScriptEngine::StatementFunction(NODE *pNode)
{
    if (pNode->vpChild.size() < 2)
        return;

    string fname = pNode->vpChild[0]->sValue;
    vector<string> params;
    NODE *pParamList = NULL;
    if (pNode->vpChild.size() >= 2)
        pParamList = pNode->vpChild[1];
    if (pParamList)
    {
        params.push_back(pParamList->sValue);
        for (auto c : pParamList->vpChild)
            params.push_back(c->sValue);
    }
    NODE *pBody = NULL;
    if (pNode->vpChild.size() >= 3)
        pBody = pNode->vpChild[2];

    auto it = find_if(m_vFunc.begin(), m_vFunc.end(), [&](const FUNCSAVETYPE &f)
                      { return f.sName == fname; });
    if (it == m_vFunc.end())
    {
        FUNCSAVETYPE f;
        f.sName = fname;
        f.params = params;
        f.pBody = pBody;
        f.nScoop = m_nScoopCount;
        m_vFunc.push_back(f);
    }
    else
    {
        it->params = params;
        it->pBody = pBody;
    }
}
void ScriptEngine::StatementWrite(NODE *pNode)
{
    for (auto a : pNode->vpChild)
    {
        Visit(a);
        cout << a->pData->sValue << endl;
    }
}
void ScriptEngine::StatementWhile(NODE *pNode)
{
    BeginScoop();
    Visit(pNode->vpChild[0]);
    while (stoi(pNode->vpChild[0]->pData->sValue))
    {
        Visit(pNode->vpChild[0]);
        Visit(pNode->vpChild[1]);
    }
    EndScoop();
}

void ScriptEngine::StatementLet(NODE *pNode)
{
    if (pNode->vpChild.size() < 2)
        return;

    string varName = pNode->vpChild[0]->sValue;

    Visit(pNode->vpChild[1]);

    if (pNode->vpChild[1]->pData == NULL)
    {
        cout << "Error: Expression has no result for variable " << varName << endl;
        return;
    }

    string value = pNode->vpChild[1]->pData->sValue;

    auto it = find_if(m_vVar.begin(), m_vVar.end(),
                      [&](const VARSAVETYPE &var)
                      { return var.sName == varName; });

    if (it == m_vVar.end())
    {
        VARSAVETYPE var;
        var.sName = varName;
        var.sValue = value;
        var.eVarType = pNode->vpChild[1]->pData->eVarType; // TODO:Correct type detection(Finished)
        var.nScoop = m_nScoopCount;
        m_vVar.push_back(var);
    }
    else
    {
        it->sValue = value;
    }
}

void ScriptEngine::StatementIf(NODE *pNode)
{
    BeginScoop();
    if (pNode->vpChild.size() < 2)
    {
        return;
    }
    Visit(pNode->vpChild[0]);
    if (pNode->vpChild[0]->pData != NULL)
    {
        VARSAVETYPE *pTemp = pNode->vpChild[0]->pData;
        if (stoi(pTemp->sValue) != 0)
        {
            Visit(pNode->vpChild[1]);
        }
    }
    EndScoop();
}

void ScriptEngine::StatementExp(NODE *pNode)
{
    if (pNode->vpChild.empty())
    {
        return;
    }
    for (auto a : pNode->vpChild)
    {
        Visit(a);
    }
    if (pNode->vpChild.size() == 3)
    {
        NODE *pLeft = pNode->vpChild[0];
        NODE *pOpNode = pNode->vpChild[2];
        NODE *pRight = pNode->vpChild[1];
        string sOp = pOpNode->sValue;
        string sLeftVal = pLeft->pData->sValue;
        string sRightVal = pRight->pData->sValue;
        string sResult;

        if (pLeft->pData && pRight->pData && pOpNode->vpChild.empty())
        {
            // bool bLeftIsNum = IsNumber(sLeftVal);
            // bool bRightIsNum = IsNumber(sRightVal);
            bool bLeftIsNum = pLeft->pData->eVarType == NUM;
            bool bRightIsNum = pRight->pData->eVarType == NUM;
            // num
            if (bLeftIsNum && bRightIsNum &&
                (sOp == "+" || sOp == "-" || sOp == "*" || sOp == "/"))
            {
                double l = stod(sLeftVal);
                double r = stod(sRightVal);

                if (sOp == "+")
                    sResult = to_string(l + r);
                else if (sOp == "-")
                    sResult = to_string(l - r);
                else if (sOp == "*")
                    sResult = to_string(l * r);
                else if (sOp == "/")
                {
                    if (fabs(r) < 1e-9)
                        sResult = "Error: Division by zero";
                    else
                        sResult = to_string(l / r);
                }
            }
            // str
            else if (sOp == "+")
            {
                string sLeftActual = sLeftVal;
                string sRightActual = sRightVal;
                sResult = sLeftActual + sRightActual;
            }
            else if (sOp == "==" || sOp == "!=" || sOp == "<" || sOp == ">" || sOp == "<=" || sOp == ">=")
            {
                string sLeftCompare = sLeftVal;
                string sRightCompare = sRightVal;

                if (!bLeftIsNum)
                {
                    for (auto a : m_vVar)
                    {
                        if (a.sName == sLeftVal)
                        {
                            sLeftCompare = a.sValue;
                            break;
                        }
                    }
                }
                if (!bRightIsNum)
                {
                    for (auto a : m_vVar)
                    {
                        if (a.sName == sRightVal)
                        {
                            sRightCompare = a.sValue;
                            break;
                        }
                    }
                }

                sResult = CompareValues(sOp, sLeftCompare, sRightCompare) ? "1" : "0";
            }
        }

        if (!sResult.empty())
        {
            pNode->pData = CreateVarData(sResult);
            // pNode->eVarType = pNode->pData->eVarType;
        }
    }
    return;
}

void ScriptEngine::StatementUnknown(NODE *pNode)
{
    // function call: node has children and name matches a defined function
    if (!pNode->vpChild.empty())
    {
        auto it = find_if(m_vFunc.begin(), m_vFunc.end(), [&](const FUNCSAVETYPE &f)
                          { return f.sName == pNode->sValue; });
        if (it != m_vFunc.end())
        {
            // evaluate arguments
            vector<string> argVals;
            for (auto arg : pNode->vpChild)
            {
                Visit(arg);
                if (arg->pData)
                    argVals.push_back(arg->pData->sValue);
                else
                    argVals.push_back(string(""));
            }

            // bind parameters
            BeginScoop();
            for (size_t i = 0; i < it->params.size(); ++i)
            {
                VARSAVETYPE var;
                var.sName = it->params[i];
                var.sValue = i < argVals.size() ? argVals[i] : string("");
                var.nScoop = m_nScoopCount;
                m_vVar.push_back(var);
            }

            // execute body
            if (it->pBody)
            {
                Visit(it->pBody);
                if (it->pBody->pData)
                    pNode->pData = CreateVarData(it->pBody->pData->sValue);
            }
            EndScoop();
            return;
        }
    }

    if (pNode->vpChild.empty()) // single variable or literal
    {
        // if (!IsNumber(pNode->sValue))
        // {
        //     if (pNode->sValue.size() >= 2 &&
        //         pNode->sValue[0] == '"' && pNode->sValue.back() == '"')
        //     {
        //         // remove quota
        //         string strValue = pNode->sValue.substr(1, pNode->sValue.size() - 2);
        //         pNode->pData = CreateVarData(strValue);
        //         return;
        //     }

        for (auto a : m_vVar)
        {
            if (a.sName == pNode->sValue)
            {
                // VARSAVETYPE aCopy = a; // Create a copy to avoid dangling pointer issues
                // pNode->pData = &aCopy; // CreateVarData(a.sValue);
                pNode->pData = CreateVarData(a.sValue);
                return;
            }
        }
        // undefined
        pNode->pData = CreateVarData(pNode->sValue);
        //}
        // else
        // {
        //     pNode->pData = CreateVarData(pNode->sValue);
        // }
    }
}
void ScriptEngine::StatementArraySet(NODE *pNode)
{
    auto it = find_if(m_vArray.begin(), m_vArray.end(), [&](const ARRAYSAVETYPE &arr)
                      { return arr.sName == pNode->vpChild[0]->sValue; });
    if (it != m_vArray.end())
    {
        Visit(pNode->vpChild[1]);
        Visit(pNode->vpChild[2]);
        if (pNode->vpChild[1]->pData && pNode->vpChild[2]->pData)
        {
            size_t index = stoi(pNode->vpChild[1]->pData->sValue);
            if (index >= 0)
            {
                VARTYPE v = pNode->vpChild[2]->pData->eVarType;
                // If array element type is not yet set, set it on first insertion
                if (it->eElemType == ANY && it->values.empty())
                {
                    it->eElemType = v;
                }
                // Enforce type check
                if (v != it->eElemType)
                {
                    cout << "Error: Type mismatch assigning to array " << it->sName << endl;
                    return;
                }

                if (index < it->values.size())
                {
                    it->values[index] = pNode->vpChild[2]->pData->sValue;
                }
                else
                {
                    it->values.push_back(pNode->vpChild[2]->pData->sValue);
                }
            }
        }
    }
}
void ScriptEngine::StatementArrayGet(NODE *pNode)
{
    auto it = find_if(m_vArray.begin(), m_vArray.end(), [&](const ARRAYSAVETYPE &arr)
                      { return arr.sName == pNode->vpChild[0]->sValue; });
    Visit(pNode->vpChild[1]);
    if (it != m_vArray.end())
    {
        size_t index = stoi(pNode->vpChild[1]->pData->sValue);
        if (index >= 0 && index < it->values.size())
        {
            pNode->pData = CreateVarData(it->values[index]);
        }
        else
        {
            pNode->pData = CreateVarData("Error: Array index out of bounds");
        }
    }
    else
    {
        pNode->pData = CreateVarData("Error: Array not found");
    }
}
void ScriptEngine::StatementArrayNew(NODE *pNode)
{
    if (pNode->vpChild.empty())
        return;
    string arrayName = pNode->vpChild[0]->sValue;
    m_vArray.push_back({arrayName, {}, ANY, m_nScoopCount});
}
void ScriptEngine::StatementArrayDel(NODE *pNode)
{
    if (pNode->vpChild.empty())
        return;
    string arrayName = pNode->vpChild[0]->sValue;
    auto it = find_if(m_vArray.begin(), m_vArray.end(), [&](const ARRAYSAVETYPE &arr)
                      { return arr.sName == arrayName; });
    Visit(pNode->vpChild[1]);
    if (it != m_vArray.end())
    {
        size_t index = stoi(pNode->vpChild[1]->pData->sValue);
        if (index >= 0 && index < it->values.size())
        {
            it->values.erase(it->values.begin() + index, it->values.begin() + index + 1);
        }
    }
}
void ScriptEngine::StatementConvert(NODE *pNode)
{
    if (pNode->vpChild.empty())
        return;
    if (pNode->vpChild.size() < 2)
        return;
    Visit(pNode->vpChild[1]);
    string targetType = pNode->vpChild[0]->sValue; // expected "NUM" or "STR"
    VARSAVETYPE *v = new VARSAVETYPE;
    v = pNode->vpChild[1]->pData;
    if (targetType == "STR")
    {
        v->eVarType = STR;
    }
    else if (targetType == "NUM")
    {
        v->eVarType = NUM;
    }
    else
    {
        cout << "Error: Invalid target type for convert: " << targetType << endl;
        return;
    }
    pNode->pData = v;
}
void ScriptEngine::Visit(NODE *pNode)
{
    if (!pNode)
    {
        return;
    }
    switch (CheckCtrlType(pNode->sValue))
    {
    case IF:
        StatementIf(pNode);
        break;
    case FOR:
        StatementFor(pNode);
        break;
    case FUNCTION:
        StatementFunction(pNode);
        break;
    case EXP:
        StatementExp(pNode);
        break;
    case WRITE:
        StatementWrite(pNode);
        break;
    case PROGRAM:
        StatementProgram(pNode);
        break;
    case LET:
        StatementLet(pNode);
        break;
    case WHILE:
        StatementWhile(pNode);
        break;
    case ARRAYSET:
        StatementArraySet(pNode);
        break;
    case ARRAYGET:
        StatementArrayGet(pNode);
        break;
    case ARRAYNEW:
        StatementArrayNew(pNode);
        break;
    case ARRAYDEL:
        StatementArrayDel(pNode);
        break;
    case CONVERT:
        StatementConvert(pNode);
        break;
    default:
        StatementUnknown(pNode);
        break;
    }
}
void ScriptEngine::PrintTree(NODE *pNode, int nDepth)
{
    if (!pNode)
        return;

    for (int i = 0; i < nDepth; i++)
        cout << "  ";

    cout << pNode->sValue << endl;

    for (auto pChild : pNode->vpChild)
    {
        PrintTree(pChild, nDepth + 1);
    }
}
void ScriptEngine::ReadFile(string filename)
{
    ifstream file(filename, ios::ate);
    if (file.is_open())
    {
        streamsize size = file.tellg();
        file.seekg(0, ios::beg);
        string content(static_cast<string::size_type>(size), ' ');
        file.read(&content[0], size);
        cout << content << endl;
        m_sCode = content;
        file.close();
    }
}
void ScriptEngine::Run(const string &filename)
{
    ReadFile(filename);
    vector<string> vsCodeData = Slice(m_sCode);
    m_pHead = new NODE;
    m_pHead->sValue = "program";
    auto begin = chrono::high_resolution_clock::now();
    BuildTree(m_pHead, vsCodeData);
    PrintTree(m_pHead);
    Visit(m_pHead);
    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);
    printf("Time measured: %.3f seconds.\n", elapsed.count() * 1e-9);
}
