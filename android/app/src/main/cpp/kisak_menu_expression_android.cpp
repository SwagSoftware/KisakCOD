#include "kisak_menu_expression_android.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <unordered_map>

#include "kisak_menu_state_android.h"

namespace {
// operationEnum (src/ui/ui_shared.h).
enum : int {
    kOpNoop = 0x0,
    kOpRightParen = 0x1,
    kOpMultiply = 0x2,
    kOpDivide = 0x3,
    kOpModulus = 0x4,
    kOpAdd = 0x5,
    kOpSubtract = 0x6,
    kOpNot = 0x7,
    kOpLessThan = 0x8,
    kOpLessThanEqualTo = 0x9,
    kOpGreaterThan = 0xA,
    kOpGreaterThanEqualTo = 0xB,
    kOpEquals = 0xC,
    kOpNotEqual = 0xD,
    kOpAnd = 0xE,
    kOpOr = 0xF,
    kOpLeftParen = 0x10,
    kOpComma = 0x11,
    kOpBitwiseAnd = 0x12,
    kOpBitwiseOr = 0x13,
    kOpBitwiseNot = 0x14,
    kOpBitShiftLeft = 0x15,
    kOpBitShiftRight = 0x16,
    kOpSin = 0x17,
    kOpCos = 0x18,
    kOpMin = 0x19,
    kOpMax = 0x1A,
    kOpMilliseconds = 0x1B,
    kOpDvarInt = 0x1C,
    kOpDvarBool = 0x1D,
    kOpDvarFloat = 0x1E,
    kOpDvarString = 0x1F,
    kOpUiActive = 0x21,
    kOpSecondsAsTime = 0x37,
    kOpTableLookup = 0x38,
    kOpLocalizeString = 0x39,
    kOpSecondsAsCountdown = 0x3F,
    kOpToInt = 0x41,
    kOpToString = 0x42,
    kOpToFloat = 0x43,
    kOpKeyBinding = 0x4B,
    kNumOperators = 0x51,
};

// s_operatorPrecedence (ui_expressions.cpp:19) — lower value binds tighter;
// 5 marks function calls (only a right paren pops them), 99 the left paren.
constexpr int kFunctionPrecedence = 5;
int Precedence(int op) {
    static const int table[] = {
        0x7FFFFFFF, 0, 11, 11, 11, 13, 13, 9, 15, 15, 15, 15, 16, 16, 25, 25,
        99, 80, 17, 18, 9, 14, 14,
    };
    if (op >= 0 && op < static_cast<int>(sizeof(table) / sizeof(table[0]))) {
        return table[op];
    }
    return kFunctionPrecedence;
}

bool IsOpAssociative(int op) {
    return op < kOpDivide || (op > kOpModulus && op != kOpSubtract);
}

bool OpPairsWithRightParen(int op) {
    return (op >= kOpSin && op <= kNumOperators) || op == kOpLeftParen;
}

// One serialized expressionEntry (12 bytes: type, then op or Operand).
struct Entry {
    bool isOperand = false;
    int op = kOpNoop;
    KisakExprValue operand;
};

using OperandList = std::vector<KisakExprValue>;
constexpr size_t kMaxListOperands = 10;

std::mutex g_dvarMutex;
std::unordered_map<std::string, std::string> g_uiDvars;
std::unordered_map<std::string, KisakExprValue> g_uiLocalVars;

std::string LocalizeKey(const KisakExprEnv& env, const std::string& key) {
    if (env.zones != nullptr) {
        for (const auto& zone : *env.zones) {
            for (const KisakLoadedLocalize& entry : zone->localizeEntries) {
                if (entry.name == key) {
                    return entry.value;
                }
            }
        }
    }
    return key;
}

// StringTable_Lookup over serialized StringTable assets (16 bytes: name,
// columnCount, rowCount, values).
std::string TableLookup(
    const KisakExprEnv& env,
    const std::string& tableName,
    int comparisonColumn,
    const std::string& value,
    int valueColumn
) {
    if (env.zones == nullptr) {
        return {};
    }
    for (const auto& zone : *env.zones) {
        KisakZoneView view(*zone);
        for (const auto& [type, ref] : zone->assetRefs) {
            if (type != 0x20 || !view.ValidRef(ref, 16)) { // stringtable
                continue;
            }
            if (view.StrAt(ref, 0) != tableName) {
                continue;
            }
            const int columnCount = static_cast<int>(view.U32(ref, 4));
            const int rowCount = static_cast<int>(view.U32(ref, 8));
            const uint32_t values = view.U32(ref, 12);
            if (columnCount <= 0 || rowCount <= 0
                || comparisonColumn < 0 || comparisonColumn >= columnCount
                || valueColumn < 0 || valueColumn >= columnCount
                || !view.ValidRef(values, static_cast<uint32_t>(columnCount * rowCount) * 4)) {
                return {};
            }
            for (int row = 0; row < rowCount; ++row) {
                const std::string cell = view.StrAt(values, (row * columnCount + comparisonColumn) * 4);
                if (strcasecmp(cell.c_str(), value.c_str()) == 0) {
                    return view.StrAt(values, (row * columnCount + valueColumn) * 4);
                }
            }
            return {};
        }
    }
    return {};
}

// LocalizeString op: string operands are "@KEY" references (leading char
// dropped, then localized); numeric operands append verbatim.
KisakExprValue RunLocalizeString(const KisakExprEnv& env, const OperandList& list) {
    std::string out;
    for (const KisakExprValue& operand : list) {
        if (operand.type == KisakExprValue::Type::String) {
            if (operand.s.size() <= 1) {
                continue;
            }
            out += LocalizeKey(env, operand.s.substr(1));
        } else {
            out += operand.AsString();
        }
    }
    return KisakExprValue::MakeString(std::move(out));
}

KisakExprValue SecondsAsTime(int seconds) {
    char buffer[32];
    if (seconds >= 3600) {
        snprintf(buffer, sizeof(buffer), "%d:%02d:%02d", seconds / 3600, seconds / 60 % 60, seconds % 60);
    } else {
        snprintf(buffer, sizeof(buffer), "%d:%02d", seconds / 60, seconds % 60);
    }
    return KisakExprValue::MakeString(buffer);
}

// GetOperationFunction semantics: string operands fall back to their numeric
// value unless both sides are strings under a comparison operator.
KisakExprValue RunLogicOp(int op, const KisakExprValue& a, const KisakExprValue& b) {
    const bool bothStrings = a.type == KisakExprValue::Type::String
        && b.type == KisakExprValue::Type::String;
    if (bothStrings) {
        const int cmp = strcmp(a.s.c_str(), b.s.c_str());
        switch (op) {
            case kOpEquals: return KisakExprValue::MakeInt(cmp == 0);
            case kOpNotEqual: return KisakExprValue::MakeInt(cmp != 0);
            case kOpAdd: return KisakExprValue::MakeString(a.s + b.s);
            default: break;
        }
    }
    const bool floats = a.type == KisakExprValue::Type::Float
        || b.type == KisakExprValue::Type::Float;
    if (floats) {
        const float x = a.AsFloat();
        const float y = b.AsFloat();
        switch (op) {
            case kOpMultiply: return KisakExprValue::MakeFloat(x * y);
            case kOpDivide: return KisakExprValue::MakeFloat(y != 0 ? x / y : 0);
            case kOpModulus: return KisakExprValue::MakeInt(0);
            case kOpAdd: return KisakExprValue::MakeFloat(x + y);
            case kOpSubtract: return KisakExprValue::MakeFloat(x - y);
            case kOpLessThan: return KisakExprValue::MakeInt(x < y);
            case kOpLessThanEqualTo: return KisakExprValue::MakeInt(x <= y);
            case kOpGreaterThan: return KisakExprValue::MakeInt(x > y);
            case kOpGreaterThanEqualTo: return KisakExprValue::MakeInt(x >= y);
            case kOpEquals: return KisakExprValue::MakeInt(x == y);
            case kOpNotEqual: return KisakExprValue::MakeInt(x != y);
            default: break;
        }
    }
    const int x = a.AsInt();
    const int y = b.AsInt();
    switch (op) {
        case kOpMultiply: return KisakExprValue::MakeInt(x * y);
        case kOpDivide: return KisakExprValue::MakeInt(y != 0 ? x / y : 0);
        case kOpModulus: return KisakExprValue::MakeInt(y != 0 ? x % y : 0);
        case kOpAdd: return KisakExprValue::MakeInt(x + y);
        case kOpSubtract: return KisakExprValue::MakeInt(x - y);
        case kOpLessThan: return KisakExprValue::MakeInt(x < y);
        case kOpLessThanEqualTo: return KisakExprValue::MakeInt(x <= y);
        case kOpGreaterThan: return KisakExprValue::MakeInt(x > y);
        case kOpGreaterThanEqualTo: return KisakExprValue::MakeInt(x >= y);
        case kOpEquals: return KisakExprValue::MakeInt(x == y);
        case kOpNotEqual: return KisakExprValue::MakeInt(x != y);
        case kOpAnd: return KisakExprValue::MakeInt(x != 0 && y != 0);
        case kOpOr: return KisakExprValue::MakeInt(x != 0 || y != 0);
        case kOpBitwiseAnd: return KisakExprValue::MakeInt(x & y);
        case kOpBitwiseOr: return KisakExprValue::MakeInt(x | y);
        case kOpBitShiftLeft: return KisakExprValue::MakeInt(x << y);
        case kOpBitShiftRight: return KisakExprValue::MakeInt(x >> y);
        default: break;
    }
    return KisakExprValue::MakeInt(0);
}

struct Evaluator {
    const KisakExprEnv& env;
    std::vector<int> opStack;
    std::vector<OperandList> dataStack;
    bool failed = false;

    explicit Evaluator(const KisakExprEnv& e) : env(e) {}

    void PushValue(KisakExprValue value) {
        dataStack.push_back(OperandList{std::move(value)});
    }

    bool PopValue(KisakExprValue* out) {
        if (dataStack.empty() || dataStack.back().size() != 1) {
            failed = true;
            *out = KisakExprValue::MakeInt(0);
            if (!dataStack.empty()) {
                dataStack.pop_back();
            }
            return false;
        }
        *out = std::move(dataStack.back().front());
        dataStack.pop_back();
        return true;
    }

    bool PopList(OperandList* out) {
        if (dataStack.empty()) {
            failed = true;
            out->assign(1, KisakExprValue::MakeInt(0));
            return false;
        }
        *out = std::move(dataStack.back());
        dataStack.pop_back();
        return true;
    }

    // Function operators that read exactly one operand.
    static bool IsUnaryFunction(int op) {
        switch (op) {
            case kOpSin:
            case kOpCos:
            case kOpDvarInt:
            case kOpDvarBool:
            case kOpDvarFloat:
            case kOpDvarString:
            case 0x20: // stat
            case 0x26: // playerfield
            case 0x28: // teamfield
            case 0x29: // otherteamfield
            case 0x2A: // marinesfield
            case 0x2B: // opforfield
            case 0x2C: // menuisopen
            case 0x34: // weaplockblink
            case kOpSecondsAsTime:
            case 0x3A: // localvarint
            case 0x3B: // localvarbool
            case 0x3C: // localvarfloat
            case 0x3D: // localvarstring
            case kOpSecondsAsCountdown:
            case 0x40: // gamemsgwndactive
            case kOpToInt:
            case kOpToString:
            case kOpToFloat:
            case 0x47: // score
            case kOpKeyBinding:
            case 0x4C: // actionslotusable
            case 0x4D: // hudfade
                return true;
            default:
                return false;
        }
    }

    static bool IsListFunction(int op) {
        switch (op) {
            case kOpMin:
            case kOpMax:
            case kOpTableLookup:
            case kOpLocalizeString:
            case 0x4A: // statrangebitsset
                return true;
            default:
                return false;
        }
    }

    KisakExprValue RunUnaryFunction(int op, const KisakExprValue& arg) {
        switch (op) {
            case kOpSin: return KisakExprValue::MakeFloat(sinf(arg.AsFloat()));
            case kOpCos: return KisakExprValue::MakeFloat(cosf(arg.AsFloat()));
            case kOpDvarInt:
                return KisakExprValue::MakeInt(atoi(GetKisakUiDvar(arg.AsString()).c_str()));
            case kOpDvarBool:
                return KisakExprValue::MakeInt(atoi(GetKisakUiDvar(arg.AsString()).c_str()) != 0);
            case kOpDvarFloat:
                return KisakExprValue::MakeFloat(
                    static_cast<float>(atof(GetKisakUiDvar(arg.AsString()).c_str())));
            case kOpDvarString:
                return KisakExprValue::MakeString(GetKisakUiDvar(arg.AsString()));
            case kOpSecondsAsTime:
            case kOpSecondsAsCountdown:
                return SecondsAsTime(arg.AsInt());
            case kOpToInt: return KisakExprValue::MakeInt(arg.AsInt());
            case kOpToString: return KisakExprValue::MakeString(arg.AsString());
            case kOpToFloat: return KisakExprValue::MakeFloat(arg.AsFloat());
            case kOpKeyBinding:
                return KisakExprValue::MakeString(KisakMenuCommandBindingText(arg.AsString()));
            case 0x3A: // localvarint
            case 0x3B: // localvarbool
                return KisakExprValue::MakeInt(GetKisakUiLocalVar(arg.AsString()).AsInt());
            case 0x3C: // localvarfloat
                return KisakExprValue::MakeFloat(GetKisakUiLocalVar(arg.AsString()).AsFloat());
            case 0x3D: // localvarstring
                return KisakExprValue::MakeString(GetKisakUiLocalVar(arg.AsString()).AsString());
            default:
                return KisakExprValue::MakeInt(0);
        }
    }

    KisakExprValue RunListFunction(int op, const OperandList& list) {
        switch (op) {
            case kOpMin:
            case kOpMax: {
                if (list.empty()) {
                    return KisakExprValue::MakeInt(0);
                }
                float best = list.front().AsFloat();
                for (const KisakExprValue& value : list) {
                    const float candidate = value.AsFloat();
                    best = op == kOpMin ? std::min(best, candidate) : std::max(best, candidate);
                }
                return KisakExprValue::MakeFloat(best);
            }
            case kOpTableLookup:
                if (list.size() == 4) {
                    return KisakExprValue::MakeString(TableLookup(
                        env,
                        list[0].AsString(),
                        list[1].AsInt(),
                        list[2].AsString(),
                        list[3].AsInt()
                    ));
                }
                return KisakExprValue::MakeString("");
            case kOpLocalizeString:
                return RunLocalizeString(env, list);
            default:
                return KisakExprValue::MakeInt(0);
        }
    }

    // RunOp (ui_expressions.cpp:629): pops one operator and executes it.
    void RunOp() {
        if (opStack.empty()) {
            failed = true;
            return;
        }
        const int op = opStack.back();
        opStack.pop_back();

        if (op == kOpRightParen) {
            int last = kOpNoop;
            while (!opStack.empty()) {
                last = opStack.back();
                RunOp();
                if (OpPairsWithRightParen(last)) {
                    break;
                }
            }
            if (!OpPairsWithRightParen(last)) {
                failed = true;
            }
            return;
        }
        if (op == kOpLeftParen || op == kOpNoop) {
            return;
        }
        if (op == kOpComma) {
            OperandList list2;
            OperandList list1;
            if (PopList(&list2) && PopList(&list1)) {
                if (list1.size() + list2.size() > kMaxListOperands) {
                    failed = true;
                    PushValue(KisakExprValue::MakeInt(0));
                    return;
                }
                list1.insert(list1.end(), list2.begin(), list2.end());
                dataStack.push_back(std::move(list1));
            }
            return;
        }
        if (op == kOpNot || op == kOpBitwiseNot) {
            KisakExprValue value;
            PopValue(&value);
            PushValue(KisakExprValue::MakeInt(
                op == kOpNot ? value.AsInt() == 0 : ~value.AsInt()));
            return;
        }
        if (op == kOpSubtract) {
            // Unary negate when only one operand is on the stack.
            KisakExprValue rhs;
            if (!PopValue(&rhs)) {
                return;
            }
            if (dataStack.empty()) {
                if (rhs.type == KisakExprValue::Type::Float) {
                    rhs.f = -rhs.f;
                } else {
                    rhs = KisakExprValue::MakeInt(-rhs.AsInt());
                }
                PushValue(std::move(rhs));
                return;
            }
            KisakExprValue lhs;
            PopValue(&lhs);
            PushValue(RunLogicOp(op, lhs, rhs));
            return;
        }
        if (op >= kOpMultiply && op <= kOpBitShiftRight) {
            KisakExprValue rhs;
            KisakExprValue lhs;
            PopValue(&rhs);
            PopValue(&lhs);
            PushValue(RunLogicOp(op, lhs, rhs));
            return;
        }
        if (op == kOpMilliseconds) {
            PushValue(KisakExprValue::MakeInt(static_cast<int>(env.milliseconds)));
            return;
        }
        if (op == kOpUiActive) {
            PushValue(KisakExprValue::MakeInt(1));
            return;
        }
        if (IsUnaryFunction(op)) {
            KisakExprValue arg;
            PopValue(&arg);
            PushValue(RunUnaryFunction(op, arg));
            return;
        }
        if (IsListFunction(op)) {
            OperandList list;
            PopList(&list);
            PushValue(RunListFunction(op, list));
            return;
        }
        // Zero-argument game-state operators (uiactive handled above): 0.
        PushValue(KisakExprValue::MakeInt(0));
    }

    // RunHigherPriorityOperators (ui_expressions.cpp:2547).
    void RunHigherPriority(int op) {
        while (!opStack.empty()) {
            const int top = opStack.back();
            if ((Precedence(top) >= Precedence(op)
                 || (Precedence(top) == kFunctionPrecedence && op != kOpRightParen))
                && (IsOpAssociative(op) || top != op)) {
                break;
            }
            RunOp();
        }
    }

    bool Evaluate(const std::vector<Entry>& entries, KisakExprValue* result) {
        for (const Entry& entry : entries) {
            if (entry.isOperand) {
                if (dataStack.size() >= 60) {
                    return false;
                }
                PushValue(entry.operand);
            } else {
                if (entry.op != kOpLeftParen) {
                    RunHigherPriority(entry.op);
                }
                if (opStack.size() >= 60) {
                    return false;
                }
                opStack.push_back(entry.op);
            }
        }
        while (!opStack.empty()) {
            RunOp();
        }
        if (failed || dataStack.size() != 1 || dataStack.front().size() != 1) {
            return false;
        }
        *result = dataStack.front().front();
        return true;
    }
};

bool ReadEntries(const KisakZoneView& view, uint32_t stmtRef, std::vector<Entry>* out) {
    const uint32_t numEntries = view.U32(stmtRef, 0);
    const uint32_t entriesRef = view.U32(stmtRef, 4);
    if (numEntries == 0 || numEntries > 200 || !view.ValidRef(entriesRef, numEntries * 4)) {
        return false;
    }
    out->reserve(numEntries);
    for (uint32_t i = 0; i < numEntries; ++i) {
        const uint32_t entryRef = view.U32(entriesRef, i * 4);
        if (!view.ValidRef(entryRef, 12)) {
            return false;
        }
        Entry entry;
        if (view.U32(entryRef, 0) == 0) { // operator
            entry.op = static_cast<int>(view.U32(entryRef, 4));
            if (entry.op < 0 || entry.op >= kNumOperators) {
                return false;
            }
        } else { // operand
            entry.isOperand = true;
            const uint32_t dataType = view.U32(entryRef, 4);
            if (dataType == 0) {
                entry.operand = KisakExprValue::MakeInt(static_cast<int>(view.U32(entryRef, 8)));
            } else if (dataType == 1) {
                entry.operand = KisakExprValue::MakeFloat(view.F32(entryRef, 8));
            } else if (dataType == 2) {
                entry.operand = KisakExprValue::MakeString(view.StrAt(entryRef, 8));
            } else {
                return false;
            }
        }
        out->push_back(std::move(entry));
    }
    return true;
}
} // namespace

KisakExprValue KisakExprValue::MakeInt(int v) {
    KisakExprValue value;
    value.type = Type::Int;
    value.i = v;
    return value;
}

KisakExprValue KisakExprValue::MakeFloat(float v) {
    KisakExprValue value;
    value.type = Type::Float;
    value.f = v;
    return value;
}

KisakExprValue KisakExprValue::MakeString(std::string v) {
    KisakExprValue value;
    value.type = Type::String;
    value.s = std::move(v);
    return value;
}

int KisakExprValue::AsInt() const {
    switch (type) {
        case Type::Int: return i;
        case Type::Float: return static_cast<int>(f);
        case Type::String: return atoi(s.c_str());
    }
    return 0;
}

float KisakExprValue::AsFloat() const {
    switch (type) {
        case Type::Int: return static_cast<float>(i);
        case Type::Float: return f;
        case Type::String: return static_cast<float>(atof(s.c_str()));
    }
    return 0.0f;
}

std::string KisakExprValue::AsString() const {
    char buffer[64];
    switch (type) {
        case Type::Int:
            snprintf(buffer, sizeof(buffer), "%i", i);
            return buffer;
        case Type::Float:
            snprintf(buffer, sizeof(buffer), "%f", f);
            return buffer;
        case Type::String:
            return s;
    }
    return {};
}

void SetKisakUiDvar(const std::string& name, const std::string& value) {
    std::lock_guard<std::mutex> lock(g_dvarMutex);
    g_uiDvars[name] = value;
}

std::string GetKisakUiDvar(const std::string& name) {
    std::lock_guard<std::mutex> lock(g_dvarMutex);
    const auto it = g_uiDvars.find(name);
    return it != g_uiDvars.end() ? it->second : std::string();
}

void SetKisakUiLocalVar(const std::string& name, const KisakExprValue& value) {
    std::lock_guard<std::mutex> lock(g_dvarMutex);
    g_uiLocalVars[name] = value;
}

KisakExprValue GetKisakUiLocalVar(const std::string& name) {
    std::lock_guard<std::mutex> lock(g_dvarMutex);
    const auto it = g_uiLocalVars.find(name);
    return it != g_uiLocalVars.end() ? it->second : KisakExprValue::MakeInt(0);
}

bool EvaluateKisakStatement(
    const KisakZoneView& view,
    uint32_t stmtRef,
    const KisakExprEnv& env,
    KisakExprValue* out
) {
    std::vector<Entry> entries;
    if (!ReadEntries(view, stmtRef, &entries)) {
        return false;
    }
    Evaluator evaluator(env);
    return evaluator.Evaluate(entries, out);
}

bool KisakStatementUsesMilliseconds(const KisakZoneView& view, uint32_t stmtRef) {
    const uint32_t numEntries = view.U32(stmtRef, 0);
    const uint32_t entriesRef = view.U32(stmtRef, 4);
    if (numEntries == 0 || numEntries > 200 || !view.ValidRef(entriesRef, numEntries * 4)) {
        return false;
    }
    for (uint32_t i = 0; i < numEntries; ++i) {
        const uint32_t entryRef = view.U32(entriesRef, i * 4);
        if (!view.ValidRef(entryRef, 12)) {
            return false;
        }
        if (view.U32(entryRef, 0) == 0
            && static_cast<int>(view.U32(entryRef, 4)) == kOpMilliseconds) {
            return true;
        }
    }
    return false;
}

bool KisakStatementTrue(const KisakZoneView& view, uint32_t stmtRef, const KisakExprEnv& env) {
    KisakExprValue result;
    return EvaluateKisakStatement(view, stmtRef, env, &result) && result.Truthy();
}

std::string KisakStatementString(
    const KisakZoneView& view,
    uint32_t stmtRef,
    const KisakExprEnv& env
) {
    KisakExprValue result;
    if (!EvaluateKisakStatement(view, stmtRef, env, &result)) {
        return {};
    }
    return result.AsString();
}
