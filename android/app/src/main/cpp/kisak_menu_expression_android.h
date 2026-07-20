#pragma once

#include <memory>
#include <string>
#include <vector>

#include "kisak_zone_loader_android.h"

// Faithful port of the IW3 UI expression evaluator
// (src/ui/ui_expressions.cpp: EvaluateExpression + RunOp) working directly on
// serialized statement_s data inside a loaded zone. Game-state operators
// (stat, playerfield, ...) resolve to safe defaults; dvar reads go through a
// small in-process dvar store.

struct KisakExprValue {
    enum class Type { Int, Float, String };
    Type type = Type::Int;
    int i = 0;
    float f = 0.0f;
    std::string s;

    static KisakExprValue MakeInt(int v);
    static KisakExprValue MakeFloat(float v);
    static KisakExprValue MakeString(std::string v);

    int AsInt() const;
    float AsFloat() const;
    std::string AsString() const;
    bool Truthy() const { return AsInt() != 0; }
};

// Zones supply localize entries and stringtables for the localizestring /
// tablelookup operators.
struct KisakExprEnv {
    const std::vector<std::shared_ptr<const KisakZoneLoadResult>>* zones = nullptr;
    uint32_t milliseconds = 0;
};

// UI dvar store used by dvarint/dvarbool/dvarfloat/dvarstring. Unknown names
// read as "".
void SetKisakUiDvar(const std::string& name, const std::string& value);
std::string GetKisakUiDvar(const std::string& name);

// UI localvar store used by localvarint/bool/float/string (menu scripts set
// them via setLocalVar*). Unknown names read as int 0.
void SetKisakUiLocalVar(const std::string& name, const KisakExprValue& value);
KisakExprValue GetKisakUiLocalVar(const std::string& name);

// stmtRef points at a serialized statement_s {numEntries, entries}. Returns
// false when the statement is empty or evaluation failed.
bool EvaluateKisakStatement(
    const KisakZoneView& view,
    uint32_t stmtRef,
    const KisakExprEnv& env,
    KisakExprValue* out
);

// True when the serialized statement reads the milliseconds operator and
// therefore needs periodic scene refreshes while visible.
bool KisakStatementUsesMilliseconds(const KisakZoneView& view, uint32_t stmtRef);

// IsExpressionTrue: empty statements evaluate to... caller decides; this
// returns false for empty/failed statements.
bool KisakStatementTrue(const KisakZoneView& view, uint32_t stmtRef, const KisakExprEnv& env);

// GetExpressionResultString: "" on empty/failed statements.
std::string KisakStatementString(
    const KisakZoneView& view,
    uint32_t stmtRef,
    const KisakExprEnv& env
);
