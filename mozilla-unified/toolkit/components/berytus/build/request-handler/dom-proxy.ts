/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { RequestHandlerParser } from "./RequestHandlerParser.js";
import { ParsedType } from "./type-parser.js";

const license = `/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
`;

export const capitalise = (str: string) => {
    if (str.length === 0) {
        return str;
    }
    return str.charAt(0).toUpperCase() + str.slice(1);
}

abstract class TypeSymbol {
    symbol: string;

    constructor(symbol: string) {
        this.symbol = symbol;
    }

    abstract movableThroughAssignment: boolean;

    /**
     * E.g., nsTArray<uint32_t>&
     */
    abstract atArgument(): string;

    /**
     * E.g., uint32_t
     */
    abstract atReturn(): string;

    /**
     * E.g., uint32_t
     */
    abstract atStruct(): string;

    /**
     * E.g., uint32_t
     */
    abstract atDefinition(): string;
}

class MemberSymbol {
    baseName: string;

    constructor(baseName: string) {
        this.baseName = baseName;
    }

    atArgument() {
        return `a${capitalise(this.baseName)}`;
    }

    atStruct() {
        return `m${capitalise(this.baseName)}`;
    }

    atInline() {
        return this.baseName;
    }
}

interface GeneratedFunction {
    functionName: string;
    functionDef: string;
    functionImpl: string;
}

interface IWrappableJSValType {
    isJsValValidFunction(): GeneratedFunction;
    importFromJsValFunction(): GeneratedFunction;
    exportToJsValFunction(): GeneratedFunction;
}

interface IDef {
    id: string;
    definition: string;
    implementation: string | undefined;
}

interface IType extends InstanceType<typeof TypeSymbol>, IWrappableJSValType, IDef {
}

abstract class BasicType extends TypeSymbol {
    get id() {
        return this.symbol;
    }

    abstract get definition(): string;

    atArgument(): string {
        return `${this.symbol}&`;
    }
    atReturn(): string {
        return `${this.symbol}`;
    }
    atDefinition(): string {
        return `${this.symbol}`;
    }
    atStruct(): string {
        return `${this.symbol}`;
    }
}

class Int32Type extends BasicType implements IType {

    constructor() {
        super('int32_t');
    }

    movableThroughAssignment = true;

    get definition(): string {
        return `${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv = aValue.isInt32();
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, int32_t& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  aRv = aValue.toInt32();
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const int32_t& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv.setInt32(aValue);
  return true;
}`
        }
    }
}

class NumberType extends BasicType implements IType {

    constructor() {
        super('double');
    }

    movableThroughAssignment = true;

    get definition(): string {
        return `${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv = aValue.isNumber() || aValue.isNumber();
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, double& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (aValue.isInt32()) {
    aRv = static_cast<double>(aValue.toInt32());
    return true;
  }
  if (NS_WARN_IF(!aValue.isNumber())) {
    return false;
  }
  aRv = aValue.toNumber();
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const double& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv.setNumber(aValue);
  return true;
}`
        }
    }
}

class VoidType extends TypeSymbol implements IType {
    constructor() {
        super('void');
    }

    movableThroughAssignment = false;

    get id() {
        return 'void';
    }

    get definition(): string {
        return ``;
    }

    get implementation(): string | undefined {
        return undefined;
    }

    atArgument(): string {
        return `${this.symbol}*`;
    }
    atReturn(): string {
        return `${this.symbol}`;
    }
    atDefinition(): string {
        return `${this.symbol}*`;
    }
    atStruct(): string {
        return `${this.symbol}*`;
    }

    isJsValValidFunction(): GeneratedFunction {
        throw new Error('Void should not be wrappable');
    }

    importFromJsValFunction(): GeneratedFunction {
        throw new Error('Void should not be wrappable');
    }
    exportToJsValFunction(): GeneratedFunction {
        throw new Error('Void should not be wrappable');
    }
}

interface EnumMember {
    name: string;
    value: string | number;
}

class UintEnumType extends BasicType implements IType {
    members: Array<EnumMember>;
    constructor(enumSymbol: string, members: Array<EnumMember>) {
        super(enumSymbol);
        if (members.find(m => typeof m.value !== 'number')) {
            throw new Error(
                `Cannot create UintEnumType for ${enumSymbol}; one `
                + `of the member's value was not a number.`
            );
        }
        this.members = members;
    }

    movableThroughAssignment = true;

    get definition(): string {
        return `enum ${this.symbol} {
  ${this.members.map(m => `${this.symbol}_${m.name} = ${m.value}`).join(",\n    ")},
  ${this.symbol}_EndGuard_
};
${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`
    }

    get implementation(): string {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(${this.symbol}::${this.symbol}_EndGuard_)) {
    return false;
  }
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.atArgument()} aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(${this.symbol}::${this.symbol}_EndGuard_)) {
    return false;
  }
  aRv = static_cast<${this.symbol}>(aValue.toInt32());
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.atArgument()} aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv.setInt32(aValue);
  return true;
}`
        }
    }
}

class StringEnumType extends BasicType implements IType {
    members: Array<EnumMember>;
    constructor(enumSymbol: string, members: Array<EnumMember>) {
        super(enumSymbol);
        if (members.find(m => typeof m.value !== 'string')) {
            throw new Error(
                `Cannot create UintEnumType for ${enumSymbol}; one `
                + `of the member's value was not a string.`
            );
        }
        this.members = members;
    }

    movableThroughAssignment = true;

    get definition(): string {
        return `struct ${this.symbol} {
  uint8_t mVal;
  ${this.symbol}() : mVal(0) {}
  ${this.symbol}(uint8_t aVal) : mVal(aVal) {}
  ${this.symbol}(${this.symbol}&& aOther) : mVal(std::move(aOther.mVal)) {}
  ${this.symbol}& operator=(${this.symbol}&& aOther) {
    mVal = std::move(aOther.mVal);
    return *this;
  }
  ${this.labeledCreatorsFunctions().map(a => a.functionDef).join("\n  ")}
  ${this.labeledCheckersFunctions().map(a => a.functionDef).join("\n  ")}
  ${this.labeledSettersFunctions().map(a => a.functionDef).join("\n  ")}
  ${this.toStringFunction().functionDef}
  ${this.fromStringFunction().functionDef}
};
${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}
`
    }

    get implementation(): string {
        return `${this.toStringFunction().functionImpl}
${this.labeledCreatorsFunctions().map(a => a.functionImpl).join("\n")}
${this.labeledCheckersFunctions().map(a => a.functionImpl).join("\n")}
${this.labeledSettersFunctions().map(a => a.functionImpl).join("\n")}
${this.fromStringFunction().functionImpl}
${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    labeledCreatorsFunctions(): GeneratedFunction[] {
        return this.members.map((m, i) => {
            const functionName = m.name;
            return {
                functionName: `${this.symbol}::${functionName}`,
                functionDef: `static ${this.symbol} ${functionName}();`,
                functionImpl: `${this.symbol} ${this.symbol}::${functionName}() {
  return ${this.symbol}(uint8_t(${i}));
}`
            }
        })
    }

    labeledCheckersFunctions(): GeneratedFunction[] {
        return this.members.map((m, i) => {
            const functionName = 'Is' + m.name;
            return {
                functionName: `${this.symbol}::${functionName}`,
                functionDef: `bool ${functionName}() const;`,
                functionImpl: `bool ${this.symbol}::${functionName}() const {
  return mVal == uint8_t(${i});
}`
            }
        })
    }

    labeledSettersFunctions(): GeneratedFunction[] {
        return this.members.map((m, i) => {
            const functionName = 'SetAs' + m.name;
            return {
                functionName: `${this.symbol}::${functionName}`,
                functionDef: `void ${functionName}();`,
                functionImpl: `void ${this.symbol}::${functionName}() {
  mVal = uint8_t(${i});
}`
            }
        })
    }

    toStringFunction(): GeneratedFunction {
        const functionName = 'ToString';
        const params = `nsString& aRetVal`;
        return {
            functionName,
            functionDef: `void ${functionName}(${params}) const;`,
            functionImpl: `void ${this.symbol}::${functionName}(${params}) const {
${this.members.map((m, i) => `  if (mVal == ${i}) {
    aRetVal.Assign(u"${m.value}"_ns);
    return;
  }`).join("\n")}
}`
        }
    }

    fromStringFunction(): GeneratedFunction {
        const functionName = 'FromString';
        const params = `const nsString& aVal, ${this.symbol}& aRetVal`;
        return {
            functionName: `${this.symbol}::${functionName}`,
            functionDef: `static bool ${functionName}(${params});`,
            functionImpl: `bool ${this.symbol}::${functionName}(${params}) {
  ${this.members.map((m, i) => `if (aVal.Equals(u"${m.value}"_ns)) {
    aRetVal.mVal = ${i};
    return true;
  }`).join("\n")}
  return false;
}`
        }
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  ${this.symbol} e;
  aRv = ${this.fromStringFunction().functionName}(strVal, e);
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.atArgument()} aRv)
`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!${this.symbol}::${this.fromStringFunction().functionName}(strVal, aRv))) {
    return false;
  }
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.atArgument()} aValue, JS::MutableHandle<JS::Value> aRv)`
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!ToJSVal(aCx, strVal, aRv))) {
    return false;
  }
  return true;
}`
        }
    }
}

class CallbackType extends TypeSymbol implements IType {
    parameters: Array<IType>;

    constructor(parameters: Array<IType>) {
        super(`std::function<void(${parameters.map(p => p.atArgument()).join(', ')})>`);
        this.parameters = parameters;
    }

    movableThroughAssignment = false;

    get id() {
        return this.symbol;
    }

    get definition() {
        return ``;
    }

    get implementation() {
        return undefined;
    }

    atArgument(): string {
        return `${this.symbol}&&`;
    }
    atReturn(): string {
        throw new Error('Callback should not be returned');
    }
    atDefinition(): string {
        throw new Error('Callback should not be used for rvalue');
    }
    atStruct(): string {
        throw new Error('Callback should not be defined in a struct');
    }

    isJsValValidFunction(): GeneratedFunction {
        throw new Error("Callback should not be wrappable");
    }

    importFromJsValFunction(): GeneratedFunction {
        throw new Error('Callback should not be wrappable');
    }
    exportToJsValFunction(): GeneratedFunction {
        throw new Error('Callback should not be wrappable');
    }
}

class StringType extends BasicType implements IType {

    constructor() {
        super('nsString');
    }

    movableThroughAssignment = true;

    get definition() {
        return `${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv = aValue.isString();
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  JS::AutoCheckCannotGC nogc;
  JSString* str = aValue.toString();
  size_t len;
  if (!JS::StringHasLatin1Chars(str)) {
    const char16_t* buf =
      JS_GetTwoByteStringCharsAndLength(aCx, nogc, str, &len);
    len = JS::GetStringLength(str);
    aRv.Assign(buf, len);
    return true;
  }
  // Latin1/OneByte JSString.
  const JS::Latin1Char* buf =
    JS_GetLatin1StringCharsAndLength(aCx, nogc, str, &len);
  CopyASCIItoUTF16(Span(reinterpret_cast<const char*>(buf), len), aRv);
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const nsString& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.get(), aValue.Length()));
  aRv.setString(rStr);
  return true;
}`
        }
    }
}

class StaticStringType extends BasicType implements IType {
    static readonly #map: Array<string> = [];

    static createOrGetSlot(literal: string): number {
        for (let i = 0; i < StaticStringType.#map.length; i++) {
            if (StaticStringType.#map[i] === literal) {
                return i;
            }
        }
        const slot = StaticStringType.#map.length;
        StaticStringType.#map.push(literal);
        return slot;
    }

    slot: number;
    literal: string;

    constructor(literal: string) {
        const slot = StaticStringType.createOrGetSlot(literal);
        const id = `StaticString${slot}`;
        super(id);
        this.slot = slot;
        this.literal = literal;
    }

    get definition(): string {
        const isAlphaNumOnly = /^[a-zA-Z0-9]+$/.test(this.literal);
        return `class ${this.symbol} : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u${JSON.stringify(this.literal)}_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}
${isAlphaNumOnly ? `
using StaticString_${this.literal} = ${this.symbol};` : ``}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    movableThroughAssignment = true;

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(${this.symbol}::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}`
        }
    }
    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  bool rv;
  if (NS_WARN_IF(!(${this.isJsValValidFunction().functionName}(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}`
        };
    }
    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}`
        }
    }


}

class BoolType extends BasicType implements IType {

    constructor() {
        super('bool');
    }

    movableThroughAssignment = true;

    get definition() {
        return `${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv = aValue.isBoolean();
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isBoolean())) {
    return false;
  }
  aRv = aValue.toBoolean();
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const bool& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv.setBoolean(aValue);
  return true;
}`
        }
    }
}

class NullType extends BasicType implements IType {
    constructor() {
        super('JSNull');
    }

    movableThroughAssignment = true;

    get definition() {
        return `
struct JSNull {};
${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv = aValue.isNull();
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.atArgument()} aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isNull())) {
    return false;
  }
  // Nothing to do...
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.atArgument()} aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv.setNull();
  return true;
}`
        }
    }
}

class MaybeType extends TypeSymbol implements IType {
    subType: IType;

    constructor(subType: IType) {
        if (subType instanceof SafeVariantType) {
            throw new Error(
                "Maybe<T> should not wrap SafeVariant<U>. "
                + "Add NothingType to the SafeVariant subtype instead."
            );
        }
        super(`Maybe<${subType.symbol}>`);
        this.subType = subType;
    }

    movableThroughAssignment = true;

    static wrapIf(subType: IType, cond: boolean | undefined): IType {
        if (! cond) {
            return subType;
        }
        return new MaybeType(subType);
    }

    get id() {
        return this.symbol;
    }

    get definition() {
        return `${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    atArgument(): string {
        return `${this.symbol}`;
    }
    atReturn(): string {
        return `${this.symbol}`;
    }
    atDefinition(): string {
        return `${this.symbol}`;
    }
    atStruct(): string {
        return `${this.symbol}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return ${this.subType.isJsValValidFunction().functionName}(aCx, aValue, aRv);
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  aRv.emplace();
  if (NS_WARN_IF(!(${this.subType.importFromJsValFunction().functionName}(aCx, aValue, *aRv)))) {
    return false;
  }
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return ${this.subType.exportToJsValFunction().functionName}(aCx, aValue.ref(), aRv);
}`
        }
    }
}

class NothingType extends TypeSymbol implements IType {
    constructor() {
        super('Nothing');
    }

    movableThroughAssignment = true;

    get id() {
        return this.symbol;;
    }

    get definition() {
        return `${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    atArgument(): string {
        return `${this.symbol}&`;
    }
    atReturn(): string {
        return `${this.symbol}`;
    }
    atStruct(): string {
        return `${this.symbol}`;
    }
    atDefinition(): string {
        return `${this.symbol}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv = aValue.isUndefined();
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, Nothing& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isUndefined())) {
    return false;
  }
  // Nothing to do...
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const Nothing& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  aRv.setUndefined();
  return true;
}`
        }
    }
}

class SafeVariantType extends TypeSymbol implements IType {
    subTypes: Array<IType>;

    constructor(subTypes: Array<IType>) {
        const flattenedSubTypes = subTypes.reduce((prev, curr) => {
            const isUnique = (s: IType) =>
                !prev.some(p => p.id === s.id);
            if (curr instanceof SafeVariantType) {
                return [
                    ...prev,
                    ...curr.subTypes.filter(s => isUnique(s))
                ];
            }
            if (!isUnique(curr)) {
                return prev;
            }
            return [...prev, curr];
        }, [] as Array<IType>);
        super("SafeVariant<" + flattenedSubTypes.map(p => p.symbol).join(', ') + ">");
        this.subTypes = flattenedSubTypes;
    }

    movableThroughAssignment = true;

    get id() {
        return this.symbol;
    }

    get subTypesSymbols() {
        return this.subTypes.map(p => p.symbol).join(', ');
    }

    get wrapsStringsOnly() {
        for (let i = 0; i < this.subTypes.length; i++) {
            if (
                this.subTypes[i] instanceof StaticStringType
                || this.subTypes[i] instanceof StringType
            ) {
                continue;
            }
            return false;
        }
        return true;
    }

    get definition() {
        return `template<>
class ${this.symbol} {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(${this.symbol}&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<${this.subTypesSymbols}>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<${this.subTypesSymbols}> const* InternalValue() const { return mVariant; }
  mozilla::Variant<${this.subTypesSymbols}>* InternalValue() { return mVariant; }
  ${this.wrapsStringsOnly ? `
  nsString AsString() const {
    MOZ_ASSERT(mVariant);
    return mVariant->match(
        ${this.subTypes.map((st, i) => `[](${st.atArgument()} aStr) -> nsString {
          ${st instanceof StaticStringType
            ? `return aStr.GetString();`
            : `return aStr;`}
        }`).join(",\n    ")}
    );
  }` : ``}
protected:
  mozilla::Variant<${this.subTypesSymbols}>* mVariant;
};
${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    atArgument(): string {
        return `${this.symbol}&`;
    }
    atReturn(): string {
        return `${this.symbol}`;
    }
    atDefinition(): string {
        return `${this.symbol}`;
    }
    atStruct(): string {
        return `${this.symbol}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName: `${functionName}`,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  ${this.subTypes.map(st => `
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(${st.isJsValValidFunction().functionName}(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);
`).join("\n")}
  aRv = false;
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.atArgument()} aRv)`
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  ${this.subTypes.map((st, i) => `do {
    bool isValid = false;
    if (NS_WARN_IF(!(${st.isJsValValidFunction().functionName}(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      ${st instanceof ArrayBufferType ?
      `aRv.Init(VariantIndex<${i}>(), ArrayBuffer());
      if (NS_WARN_IF(!(${st.importFromJsValFunction().functionName}(aCx, aValue, (aRv.InternalValue())->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      ` :
      `${st.atDefinition()} nv;
      if (NS_WARN_IF(!(${st.importFromJsValFunction().functionName}(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<${i}>(), std::move(nv));
      return true;`}
    }
  } while (false);
  `).join("\n")}

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.atArgument()} aValue, JS::MutableHandle<JS::Value> aRv)`;
        const matcherName = `Matcher`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  struct ${matcherName} {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    ${matcherName}(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    ${this.subTypes.map(st => `
    bool operator()(const ${st.atArgument()} aVal) {
      return ${st.exportToJsValFunction().functionName}(mCx, aVal, mRv);
    }`).join("\n")}
  };
  return aValue.InternalValue()->match(${matcherName}(aCx, aRv));
}`
        }
    }
}

class ArrayType extends TypeSymbol implements IType {
    subType: IType;

    constructor(subType: IType) {
        super(`nsTArray<${subType.symbol}>`);
        this.subType = subType;
    }

    get id() {
        return this.symbol
    }

    movableThroughAssignment = true;

    get definition() {
        return `${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    atArgument(): string {
        return `${this.symbol}`;
    }
    atReturn(): string {
        return `${this.symbol}`;
    }
    atDefinition(): string {
        return `${this.symbol}`;
    }
    atStruct(): string {
        return `${this.symbol}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (!aValue.isObject()) {
    return false;
  }
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, aValue, &aRv))) {
    return false;
  }
  // TODO(berytus): What about the values inside the array?
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isArray;
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, obj, &isArray))) {
    return false;
  }
  if (NS_WARN_IF(!isArray)) {
    return false;
  }
  uint32_t length;
  if (NS_WARN_IF(!JS::GetArrayLength(aCx, obj, &length))) {
    return false;
  }
  for (uint32_t i = 0; i < length; i++) {
    JS::Rooted<JS::Value> value(aCx);

    if (NS_WARN_IF(!JS_GetElement(aCx, obj, i, &value))) {
      return false;
    }

    ${this.subType.symbol} item;
    if (NS_WARN_IF(!(${this.subType.importFromJsValFunction().functionName}(aCx, value, item)))) {
      return false;
    }
    aRv.AppendElement(std::move(item));
  }
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const ${this.subType.symbol}& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!(${this.subType.exportToJsValFunction().functionName}(aCx, item, &value)))) {
      return false;
    }
    if (NS_WARN_IF(!JS_DefineElement(aCx, array, i, value, JSPROP_ENUMERATE))) {
      return false;
    }
  }
  aRv.setObject(*array);
  return true;
}`
        }
    }
}

class ArrayBufferType extends TypeSymbol implements IType {

    constructor() {
        super('ArrayBuffer');
    }

    get id() {
        return this.symbol;
    }

    movableThroughAssignment = false;

    get definition() {
        return `${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    atArgument(): string {
        return `${this.symbol}&`;
    }
    atReturn(): string {
        return `${this.symbol}`;
    }
    atDefinition(): string {
        return `${this.symbol}`;
    }
    atStruct(): string {
        return `${this.symbol}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  aRv = JS::IsArrayBufferObject(obj);
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  if (NS_WARN_IF(!JS::IsArrayBufferObject(obj))) {
    return false;
  }
  size_t length = 0u;
  uint8_t* bytes = nullptr;
  JSObject* buff = JS::GetObjectAsArrayBuffer(obj, &length, &bytes);
  if (NS_WARN_IF(!buff)) {
    return false;
  }
  if (NS_WARN_IF(aRv.inited())) {
    return false;
  }
  // TODO(berytus): Check if we need to pass obj or buff
  if (NS_WARN_IF(!aRv.Init(obj))) {
    return false;
  }
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  MOZ_ASSERT(aValue.Obj()); // TODO(berytus): Remove or keep this.
  aRv.setObject(*aValue.Obj());
  return true;
}`
        }
    }
}

class ArrayBufferViewType extends TypeSymbol implements IType {

    constructor() {
        super('ArrayBufferView');
    }

    get id() {
        return this.symbol;
    }

    movableThroughAssignment = false;

    get definition() {
        return `${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    atArgument(): string {
        return `${this.symbol}&`;
    }
    atReturn(): string {
        return `${this.symbol}`;
    }
    atDefinition(): string {
        return `${this.symbol}`;
    }
    atStruct(): string {
        return `${this.symbol}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  aRv = JS_IsArrayBufferViewObject(obj);
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  if (NS_WARN_IF(!JS_IsArrayBufferViewObject(obj))) {
    return false;
  }
  bool isShared;
  JSObject* buff = JS_GetArrayBufferViewBuffer(aCx, obj, &isShared);
  if (NS_WARN_IF(!buff)) {
    return false;
  }
  if (NS_WARN_IF(aRv.inited())) {
    return false;
  }
  // TODO(berytus): Test if we need to pass the obj or buff
  if (NS_WARN_IF(!aRv.Init(obj))) {
    return false;
  }
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  MOZ_ASSERT(aValue.Obj()); // TODO(berytus): Remove or keep this.
  aRv.setObject(*aValue.Obj());
  return true;
}`
        }
    }
}

abstract class TypedMember {
    type: IType;
    member: MemberSymbol;

    constructor(type: IType, member: MemberSymbol) {
        this.type = type;
        this.member = member;
    }

    abstract toString(): string;
}

class ArgumentMember extends TypedMember {
    toString(): string {
        return this.type.atArgument() + ' ' + this.member.atArgument();
    }
}

class StructMember extends TypedMember {
    toString(): string {
        return this.type.atStruct() + ' ' + this.member.atStruct();
    }
}

class StructType extends TypeSymbol implements IType {
    members: Array<StructMember>;

    constructor(structSymbol: string, members: Array<StructMember>) {
        super(structSymbol);
        this.members = members;
    }

    get movableThroughAssignment() {
        for (let i = 0; i < this.members.length; i++) {
            if (! this.members[i].type.movableThroughAssignment) {
                return false;
            }
        }
        return true;
    }

    get id() {
        return this.symbol;
    }

    get definition() {
        return `struct ${this.symbol} {
  ${this.members.map(m => m.toString() + `;`).join("\n  ")}
  ${this.symbol}() = default;
  ${this.members.length > 0 ? `\
${this.symbol}(${this.members.map(({ member, type }) => `${type.atStruct()}&& ${member.atArgument()}`).join(", ")}) : ${this.members.map(({ member }) => `${member.atStruct()}(std::move(${member.atArgument()}))`).join(", ")} {}
  ${this.symbol}(${this.symbol}&& aOther) : ${this.members.map(({ member }) => `${member.atStruct()}(std::move(aOther.${member.atStruct()}))`).join(", ")}  {}
  ${this.movableThroughAssignment
      ? `\
${this.symbol}& operator=(${this.symbol}&& aOther) {
    ${this.members.map(({ member }) => `${member.atStruct()} = std::move(aOther.${member.atStruct()});`).join("\n  ")}
    return *this;
  }` : ""}
  ` : ""}
  ~${this.symbol}() {}
};
${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }

    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    atArgument(): string {
        return `${this.symbol}&`;
    }
    atReturn(): string {
        return `${this.symbol}`;
    }
    atStruct(): string {
        return `${this.symbol}`;
    }
    atDefinition(): string {
        return `${this.symbol}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        const hasMembers = this.members.length > 0;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
${hasMembers ? `\
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  ${this.members.map(({ type, member }) => `
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "${member.baseName}", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(${type.isJsValValidFunction().functionName}(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  ` ).join("\n")}
  aRv = true;
  return true;
` : `\
  aRv = true;
  return true;
`}

}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  ${this.members.map(({ type, member }) => `
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "${member.baseName}", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(${type.importFromJsValFunction().functionName}(aCx, propVal, aRv.${member.atStruct()})))) {
    return false;
  }
  `).join("\n")}
  return true;
}
            `
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  ${this.members.map(({ type, member }, i) => `
  JS::Rooted<JS::Value> memberVal${i}(aCx);
  ${type instanceof SafeVariantType ? `if (NS_WARN_IF(!aValue.${member.atStruct()}.Inited())) {
    return false;
  }` : ``}
  if (NS_WARN_IF(!(${type.exportToJsValFunction().functionName}(aCx, aValue.${member.atStruct()}, &memberVal${i})))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "${member.baseName}", memberVal${i}))) {
    return false;
  }
  `).join("\n")}
  aRv.setObject(*obj);
  return true;
}
`
        }
    }
}

class RecordType extends BasicType implements IType {
    readonly keyType: IType;
    readonly valueType: IType;

    constructor(keyType: IType, valueType: IType) {
        super(`Record<${keyType.symbol}, ${valueType.symbol}>`);
        this.keyType = keyType;
        this.valueType = valueType;
        if (!(keyType instanceof StringType) && !(keyType instanceof Int32Type)) {
            throw new Error("Record type only supports key type that is either a StringType or an Int32Type");
        }
    }

    movableThroughAssignment = false;

    get definition() {
        return `\
${this.isJsValValidFunction().functionDef}
${this.importFromJsValFunction().functionDef}
${this.exportToJsValFunction().functionDef}`;
    }
    get implementation() {
        return `${this.isJsValValidFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}
${this.exportToJsValFunction().functionImpl}`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = `JSValIs<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  if (aValue.isNull()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, aValue.toObjectOrNull());
  JS::Rooted<JS::IdVector> ids(aCx, JS::IdVector(aCx));
  if (NS_WARN_IF(!JS_Enumerate(aCx, obj, &ids))) {
    aRv = false;
    return false;
  }
  JS::Rooted<JS::PropertyKey> prop(aCx);
  JS::Rooted<JS::Value> val(aCx);
  for (size_t i = 0, n = ids.length(); i < n; i++) {
    prop = ids[i];
${this.keyType instanceof StringType ? `\
    if (!prop.isString()) {
      aRv = false;
      return true;
    }` : `\
    if (!prop.isInt()) {
      aRv = false;
      return true;
    }` }

    if (NS_WARN_IF(!JS_GetPropertyById(aCx, obj, prop, &val))) {
      aRv = false;
      return false;
    }
    bool isValid;
    if (NS_WARN_IF((!${this.valueType.isJsValValidFunction().functionName}(aCx, val, isValid)))) {
      aRv = false;
      return false;
    }
    if (!isValid) {
      aRv = false;
      return true;
    }
  }
  aRv = true;
  return true;
}`
        }
    }
    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, ${this.atArgument()} aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  if (NS_WARN_IF(aValue.isNull())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, aValue.toObjectOrNull());
  JS::Rooted<JS::IdVector> ids(aCx, JS::IdVector(aCx));
  if (NS_WARN_IF(!JS_Enumerate(aCx, obj, &ids))) {
    return false;
  }
  JS::Rooted<JS::PropertyKey> prop(aCx);
  JS::Rooted<JS::Value> val(aCx);
  for (size_t i = 0, n = ids.length(); i < n; i++) {
    prop = ids[i];
    ${this.symbol}::EntryType entry;
${this.keyType instanceof StringType ? `\
    if (NS_WARN_IF(!prop.isString())) {
      return false;
    }
    nsAutoJSString propName;
    if (NS_WARN_IF(!propName.init(aCx, prop))) {
      return false;
    }
    entry.mKey.Assign(propName);` : `\
    if (NS_WARN_IF(!prop.isInt())) {
      return false;
    }
    entry.mKey = prop.toInt();` }

    if (NS_WARN_IF(!JS_GetPropertyById(aCx, obj, prop, &val))) {
      return false;
    }
    if (NS_WARN_IF((!${this.valueType.importFromJsValFunction().functionName}(aCx, val, entry.mValue)))) {
      return false;
    }
    aRv.Entries().AppendElement(std::move(entry));
  }
  return true;
};`
        }
    }
    exportToJsValFunction(): GeneratedFunction {
        const functionName = `ToJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, const ${this.atArgument()} aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));
  for (const auto& entry : aValue.Entries()) {
    JS::Rooted<JS::Value> val(aCx);
    if (NS_WARN_IF((!${this.valueType.exportToJsValFunction().functionName}(aCx, entry.mValue, &val)))) {
      return false;
    }
${this.keyType instanceof StringType ? `\
    nsCString propName = NS_ConvertUTF16toUTF8(entry.mKey);
    if (NS_WARN_IF(!JS_SetProperty(aCx, obj, propName.BeginReading(), val))) {
      return false;
    }`
    : `\
    JS::Rooted<JS::PropertyKey> prop(aCx, JS::PropertyKey::Int(entry.mKey));
    if (NS_WARN_IF(!JS_SetPropertyById(aCx, obj, prop, val))) {
      return false;
    }` }
  }
  aRv.setObjectOrNull(obj);
  return true;
}`
        };
    }

}

abstract class MethodDef implements IDef {
    name: string;
    parameters: Array<ArgumentMember>;
    returnType: IType;

    constructor(name: string, parameters: Array<ArgumentMember>, returnType: IType) {
        this.name = name;
        this.parameters = parameters;
        this.returnType = returnType;
    }

    get id() {
        return this.name;
    }

    get definition() {
        return `${this.returnType.atReturn()} ${this.name}(${this.parameters.map(p => p.toString()).join(', ')});`
    }

    abstract get implementation(): string;
}

class MozPromiseType extends TypeSymbol implements IType {
    resolveType: IType;
    rejectType: IType;
    isExclusive: boolean;

    constructor(alias: string, resolveType: IType, rejectType: IType, isExclusive = true) {
        super(alias);
        this.resolveType = resolveType;
        this.rejectType = rejectType;
        this.isExclusive = isExclusive;
    }

    movableThroughAssignment = false;

    get id() {
        return this.symbol;
    }

    get definition() {
        return `using ${this.symbol} = MozPromise<${this.resolveType.atDefinition()}, ${this.rejectType.atDefinition()}, ${this.isExclusive ? 'true' : 'false'}>;`;
    }

    get implementation() {
        return undefined;
    }

    atArgument(): string {
        return `RefPtr<${this.symbol}>`;
    }
    atReturn(): string {
        return `RefPtr<${this.symbol}>`;
    }
    atStruct(): string {
        return `RefPtr<${this.symbol}>`;
    }
    atDefinition(): string {
        return `RefPtr<${this.symbol}>`;
    }

    isJsValValidFunction(): GeneratedFunction {
        throw new Error("MozPromise should not be wrapped by a JS Value.");
    }
    importFromJsValFunction(): GeneratedFunction {
        throw new Error("MozPromise should not be wrapped by a JS Value.");
    }
    exportToJsValFunction(): GeneratedFunction {
        throw new Error("MozPromise should not be wrapped by a JS Value.");
    }
}

class FailureType extends BasicType implements IType {
    constructor() {
        super('Failure');
    }

    movableThroughAssignment = true;

    get definition(): string {
        return `\
#define BERYTUS_AGENT_DEFAULT_EXCEPTION_MESSAGE nsCString("An error has occurred"_ns)
#define BERYTUS_AGENT_DEFAULT_EXCEPTION_NAME nsCString("An error has occurred"_ns)
struct ${this.symbol} {
  RefPtr<mozilla::dom::Exception> mException;

  ${this.symbol}() : ${this.symbol}(NS_ERROR_FAILURE) {}
  ${this.symbol}(nsresult res) : mException(new mozilla::dom::Exception(BERYTUS_AGENT_DEFAULT_EXCEPTION_MESSAGE, NS_ERROR_FAILURE, BERYTUS_AGENT_DEFAULT_EXCEPTION_NAME)) {}

  ${this.toErrorResultFunction().functionDef}
};
${this.importFromJsValFunction().functionDef}`
    }

    get implementation() {
        return `${this.toErrorResultFunction().functionImpl}
${this.importFromJsValFunction().functionImpl}`;
    }

    toErrorResultFunction(): GeneratedFunction {
        const functionName = 'ToErrorResult';
        return {
            get functionName(): string {
                throw new Error(
                    'functionName of Failure.ErrorResult should not be accessed'
                );
            },
            functionDef: `ErrorResult ${functionName}() const;`,
            functionImpl: `ErrorResult ${this.symbol}::${functionName}() const {
  ErrorResult rv;
  nsresult result = mException->GetResult();
    const nsCString& msg = mException->GetMessageMoz();

  if (result == NS_ERROR_ABORT) {
    rv.ThrowAbortError(
      msg.Length() == 0
        ? "Request aborted"_ns
        : msg
    );
  } else {
    rv.ThrowInvalidStateError(
      msg.Length() == 0
        ? "Request failed"_ns
        : msg
    );
  }
  return rv;
}`
        }
    }

    isJsValValidFunction(): GeneratedFunction {
        throw new Error("FailureType should not provide a IsJSValValid fnc");
    }
    importFromJsValFunction(): GeneratedFunction {
        const functionName = `FromJSVal<${this.symbol}>`;
        const funcDef = `template<>
bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  nsresult result = NS_ERROR_FAILURE;
  nsIStackFrame* stackFrame = nullptr;
  nsISupports* data = nullptr;
  nsCString msg = BERYTUS_AGENT_DEFAULT_EXCEPTION_MESSAGE;
  nsCString excpName = BERYTUS_AGENT_DEFAULT_EXCEPTION_NAME;
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  if (NS_WARN_IF(js::IsCrossCompartmentWrapper(&aValue.toObject()))) {
    return false;
  }
  JS::RootedObject errorObj(aCx, &aValue.toObject());
  JS::RootedValue messageVal(aCx);
  JS::RootedValue resultVal(aCx);
  if (JS_GetProperty(aCx, errorObj, "result", &resultVal)) {
    if (resultVal.isInt32()) {
      result = nsresult(resultVal.toInt32());
    } else if (resultVal.isDouble()) {
      result = nsresult(resultVal.toDouble());
    }
  }
  if (JS_GetProperty(aCx, errorObj, "message", &messageVal)) {
    nsString twoByteMsg;
    if (NS_WARN_IF(!FromJSVal(aCx, messageVal, twoByteMsg))) {
      return false;
    }
    // additional copy... TODO(berytus): possibly optimise this.
    msg = NS_ConvertUTF16toUTF8(twoByteMsg);
  }
  aRv.mException =
      new mozilla::dom::Exception(msg,
                                  result,
                                  excpName,
                                  stackFrame,
                                  data);
  return true;
}`
        }
    }
    exportToJsValFunction(): GeneratedFunction {
        throw new Error("FailureType should not be exportable to JS");
    }
}

class RequestHandlerMethod extends MethodDef implements IDef {
    className: string;
    group: string;
    method: string;
    declare returnType: MozPromiseType;

    constructor(className: string, group: string, method: string, parameters: Array<ArgumentMember>, outType: IType) {
        super(
            capitalise(group) + "_" + capitalise(method),
            parameters,
            new MozPromiseType(`${capitalise(group)}${capitalise(method)}Result`, outType, new FailureType(), true)
        );
        this.className = className;
        this.group = group;
        this.method = method;

    }

    get implementation() {
        const { group, method, parameters, returnType, name, className } = this;
        if (! parameters || parameters.length === 0 ) {
            return ``;
        }
        return `${returnType.atReturn()} ${className}::${name}(${parameters.map(p => p.toString()).join(', ')}) {
  RefPtr<${this.returnType.symbol}::Private> outPromise = new ${this.returnType.symbol}::Private(__func__);
  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"${group}"_ns,
                                            u"${method}"_ns,
                                            aContext,
                                            ${parameters[1] ? '&aArgs' : 'static_cast<PreliminaryRequestContext*>(nullptr)'},
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("${name}:onResolve()"));
    ${this.returnType.resolveType instanceof VoidType ? `\
void* out = nullptr;
    outPromise->Resolve(out, __func__);` : `\
${this.returnType.resolveType.symbol} out;
    if (NS_WARN_IF(!(${this.returnType.resolveType.importFromJsValFunction().functionName}(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    `}
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("${name}:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}`;
    }
}

export class UniqueArray<T extends { id: string }> extends Array<T> {
    push(...items: T[]): number {
        for (let i = 0; i < items.length; i++) {
            const foundIndex = super.findIndex(
                it => it.id === items[i].id
            );
            if (foundIndex === -1) {
                super.push(items[i]);
                continue;
            }
            super[foundIndex] = items[i];
        }
        return super.length;
    }
}

class AgentProxyGenerator {
    static className = `AgentProxy`;
    defs = new UniqueArray<IDef>();

    addMethod(
        group: string,
        method: string,
        parameters: Array<ArgumentMember>,
        outType: IType
    ) {
        const methodType = new RequestHandlerMethod(
            AgentProxyGenerator.className,
            group,
            method,
            [...parameters],
            outType
        );
        // add MozPromise's reject type type definition (FailureType)
        this.defs.push(
            methodType.returnType.rejectType
        );
        // -- MozPromise's resolve type is `outType` and the caller
        // of `addMethod` has already added its definition.
        // add MozPromise alias:
        this.defs.push(
            methodType.returnType
        );
        this.defs.push(
            methodType
        );
    }

    defineType(parsedType: ParsedType): IType {
        if (parsedType.type === "object") {
            return this.defineStruct(parsedType);
        }
        if (parsedType.type === "enum") {
            return this.defineEnum(parsedType);
        }
        if (parsedType.type === "literal") {
            switch (typeof parsedType.value) {
                case "number":
                    console.warn("Unsupported NumberLiteralType; resorting to NumberType");
                    return this.defineNumber(parsedType);
                case "string":
                    return this.defineStringLiteral(parsedType);
                case "boolean":
                    console.warn("Unsupported BooleanLiteralType; resorting to BooleanType");
                    return this.defineBoolean(parsedType);
                default:
                    throw new Error("Unrecognised type literal " + parsedType.value);
            }
        }
        if (parsedType.type === "union") {
            return this.defineUnion(parsedType);
        }
        if (parsedType.type === "Array") {
            return this.defineArray(parsedType);
        }
        if (parsedType.type === "any") {
            return new VoidType();
        }
        if (parsedType.type === "null") {
            return this.defineNull(parsedType);
        }
        if (parsedType.type === "undefined") {
            return this.defineUndefined(parsedType);
        }
        if (parsedType.type === "ArrayBuffer") {
            return this.defineArrayBuffer(parsedType);
        }
        if (parsedType.type === "ArrayBufferView") {
            return this.defineArrayBufferView(parsedType);
        }
        if (parsedType.type === "boolean") {
            return this.defineBoolean(parsedType);
        }
        if (parsedType.type === "number") {
            return this.defineNumber(parsedType);
        }
        if (parsedType.type === "string") {
            return this.defineString(parsedType);
        }
        if (parsedType.type === "pattern") {
            console.warn("Unsupported PatternType; resorting to StringType:");
            console.warn(parsedType);
            return this.defineString(parsedType);
        }
        if (parsedType.type === "record") {
            return this.defineRecord(parsedType);
        }
        throw new Error('Unsupported type ' + parsedType.type);
    }

    defineArrayBuffer(parsedType: ParsedType): IType {
        if (parsedType.type !== "ArrayBuffer") {
            throw new Error(
                "Wrong type passed to defineArrayBuffer"
            );
        }
        const abType = new ArrayBufferType();
        this.defs.push(abType);
        if (parsedType.optional) {
            const maybeType = new MaybeType(abType);
            this.defs.push(maybeType);
            return maybeType;
        }
        return abType;
    }

    defineArrayBufferView(parsedType: ParsedType): IType {
        if (parsedType.type !== "ArrayBufferView") {
            throw new Error(
                "Wrong type passed to defineArrayBufferView"
            );
        }
        const abType = new ArrayBufferViewType();
        this.defs.push(abType);
        if (parsedType.optional) {
            const maybeType = new MaybeType(abType);
            this.defs.push(maybeType);
            return maybeType;
        }
        return abType;
    }

    defineArray(parsedType: ParsedType): IType {
        if (parsedType.type !== "Array") {
            throw new Error(
                "Wrong type passed to defineArrayBuffer"
            );
        }
        const subType = this.defineType(parsedType.itemType);
        const arrayType = new ArrayType(subType);
        this.defs.push(arrayType);
        if (parsedType.optional) {
            const maybeType = new MaybeType(arrayType);
            this.defs.push(maybeType);
            return maybeType;
        }
        return arrayType;
    }

    defineString(parsedType: ParsedType): IType {
        if (
            parsedType.type !== "string"
            && parsedType.type !== "pattern"
        ) {
            throw new Error(
                "Wrong type passed to defineString"
            );
        }
        const strType = new StringType();
        this.defs.push(strType);
        if (parsedType.optional) {
            const maybeType = new MaybeType(strType);
            this.defs.push(maybeType);
            return maybeType;
        }
        return strType;
    }

    defineStringLiteral(parsedType: ParsedType): IType {
        if (
            parsedType.type !== "literal"
            || typeof parsedType.value !== "string"
        ) {
            throw new Error(
                "Wrong type passed to defineStringLiteral"
            );
        }
        const staticStrType = new StaticStringType(parsedType.value);
        this.defs.push(staticStrType);
        if (parsedType.optional) {
            const maybeType = new MaybeType(staticStrType);
            this.defs.push(maybeType);
            return maybeType;
        }
        return staticStrType;
    }

    defineNumber(parsedType: ParsedType): IType {
        if (
            parsedType.type !== "number"
            && !(
                parsedType.type === "literal"
                && typeof parsedType.value === "number"
            )
        ) {
            throw new Error(
                "Wrong type passed to defineNumber"
            );
        }
        const numType = new NumberType();
        this.defs.push(numType);
        if (parsedType.optional) {
            const maybeType = new MaybeType(numType);
            this.defs.push(maybeType);
            return maybeType;
        }
        return numType;
    }

    defineBoolean(parsedType: ParsedType): IType {
        if (
            parsedType.type !== "boolean"
            && !(
                parsedType.type === "literal"
                && typeof parsedType.value === "boolean"
            )
        ) {
            throw new Error(
                "Wrong type passed to defineBoolean"
            );
        }
        const boolType = new BoolType();
        this.defs.push(boolType);
        if (parsedType.optional) {
            const maybeType = new MaybeType(boolType);
            this.defs.push(maybeType);
            return maybeType;
        }
        return boolType;
    }

    defineNull(parsedType: ParsedType): IType {
        if (parsedType.type !== "null") {
            throw new Error(
                "Wrong type passed to defineNull"
            );
        }
        if (parsedType.optional) {
            throw new Error("Null types cannot be optional");
        }
        const nullType = new NullType();
        this.defs.push(nullType);
        return nullType;
    }

    defineUndefined(parsedType: ParsedType): IType {
        if (parsedType.type !== "undefined") {
            throw new Error(
                "Wrong type passed to defineUndefined"
            );
        }
        if (parsedType.optional) {
            throw new Error("Undefined types cannot be optional");
        }
        const undefinedType = new NothingType();
        this.defs.push(undefinedType);
        return undefinedType;
    }

    defineUnion(parsedType: ParsedType): IType {
        if (parsedType.type !== "union") {
            throw new Error(
                "Wrong type passed to defineUnion"
            );
        }
        // TODO(berytus): Group all string literals into one StringEnum
        const subTypes = parsedType.options.map(pt => this.defineType(pt));
        if (parsedType.optional) {
            const nothingType = new NothingType();
            this.defs.push(nothingType);
            subTypes.push(nothingType);
        }
        // drop-in-patch: literal types are mapped to non-literal C++
        // types. The variant type should receive duplicate sub types;
        // e.g., subTypes = [new StringType(), new StringType()].
        // Here, we filter subTypes to remove duplicates due to
        // literal types.
        const filteredSubTypes: Array<IType> = Object.values(subTypes.reduce((pre, curr, i) => {
            if (pre[curr.symbol]) {
                return pre;
            }
            pre[curr.symbol] = curr;
            return pre;
        }, {} as Record<string, IType>));
        if (filteredSubTypes.length === 1) {
            this.defs.push(filteredSubTypes[0]);
            return filteredSubTypes[0];
        }
        if (filteredSubTypes.length === 2 && filteredSubTypes.some(s => (s instanceof NothingType))) {
            const type = filteredSubTypes.find(s => !(s instanceof NothingType))!;
            this.defs.push(type);
            return type;
        }
        const variantType = new SafeVariantType(filteredSubTypes);
        this.defs.push(
            variantType
        );
        return variantType;
    }

    defineEnum(parsedType: ParsedType): IType {
        if (parsedType.type !== "enum") {
            throw new Error(
                "Wrong type passed to defineEnum"
            );
        }
        let enumType: StringEnumType | UintEnumType;
        if (typeof parsedType.choices[0].value === "number") {
            enumType = new UintEnumType(
                parsedType.alias,
                parsedType.choices
            );
        } else {
            enumType = new StringEnumType(
                parsedType.alias,
                parsedType.choices
            );
        }

        this.defs.push(enumType);
        if (parsedType.optional) {
            const maybeType = new MaybeType(enumType);
            this.defs.push(maybeType);
            return maybeType;
        }
        return enumType;
    }

    defineStruct(parsedType: ParsedType): IType {
        if (parsedType.type !== "object") {
            throw new Error(
                "Wrong type passed to defineStruct"
            );
        }
        const { properties, alias } = parsedType;
        const members: Array<StructMember> = [];
        for (const prop in properties) {
            if (prop === "request") {
                // skip Request - { requestId, requestType }
                continue;
            }
            const parsedType = properties[prop];
            const memberType = this.defineType(parsedType);
            members.push(
                new StructMember(
                    memberType,
                    new MemberSymbol(prop)
                )
            );
        }
        const structType = new StructType(
            alias,
            members
        );
        this.defs.push(structType);
        if (parsedType.optional) {
            const maybeType = new MaybeType(structType);
            this.defs.push(maybeType);
            return maybeType;
        }
        return structType;
    }

    defineRecord(parsedType: ParsedType): IType {
        if (parsedType.type !== "record") {
            throw new Error(
                "Wrong type passed to defineRecord"
            );
        }
        const keyType = this.defineType({
            type: parsedType.keyType
        });
        const valueType = this.defineType(parsedType.valueType);
        const recordType = new RecordType(
            keyType,
            valueType
        );
        this.defs.push(recordType);
        if (parsedType.optional) {
            const maybeType = new MaybeType(recordType);
            this.defs.push(maybeType);
            return maybeType;
        }
        return recordType;
    }

    generateImpl() {
        return `${license}

#include "mozilla/berytus/${AgentProxyGenerator.className}.h"
#include "js/PropertyAndElement.h"
#include "js/String.h"
#include "js/Value.h"
#include "mozilla/dom/JSActorService.h"
#include "mozilla/dom/WindowGlobalChild.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/JSWindowActorChild.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/Promise-inl.h"
#include "mozilla/dom/PromiseNativeHandler.h"

static mozilla::LazyLogModule sLogger("berytus_agent");

namespace mozilla::berytus {

NS_IMPL_CYCLE_COLLECTION(${AgentProxyGenerator.className}, mGlobal)
NS_IMPL_CYCLE_COLLECTING_ADDREF(${AgentProxyGenerator.className})
NS_IMPL_CYCLE_COLLECTING_RELEASE(${AgentProxyGenerator.className})
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(${AgentProxyGenerator.className})
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

${AgentProxyGenerator.className}::${AgentProxyGenerator.className}(
    nsIGlobalObject* aGlobal, const nsAString& aManagerId)
    : mGlobal(aGlobal), mManagerId(aManagerId), mDisabled(false) {}

${AgentProxyGenerator.className}::~${AgentProxyGenerator.className}() {}

bool ${AgentProxyGenerator.className}::IsDisabled() const {
  return mDisabled;
}

template <typename W1, typename W2>
already_AddRefed<dom::Promise> ${AgentProxyGenerator.className}::CallSendQuery(JSContext *aCx,
                                                         const nsAString & aGroup,
                                                         const nsAString &aMethod,
                                                         const W1& aReqCx,
                                                         const W2* aReqArgs,
                                                         ErrorResult& aRv) {
  JS::Rooted<JS::Value> reqArgsJS(aCx, JS::UndefinedValue());
  if (aReqArgs) {
    if (NS_WARN_IF(!ToJSVal(aCx, *aReqArgs, &reqArgsJS))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
  }
  return CallSendQuery(aCx, aGroup, aMethod, aReqCx, reqArgsJS, aRv);
}

template <typename W1>
already_AddRefed<dom::Promise> ${AgentProxyGenerator.className}::CallSendQuery(JSContext *aCx,
                                                         const nsAString & aGroup,
                                                         const nsAString &aMethod,
                                                         const W1& aReqCx,
                                                         JS::Handle<JS::Value> aReqArgsJs,
                                                         ErrorResult& aRv) {
  MOZ_LOG(sLogger, LogLevel::Info, ("SendQuery %s:%s", NS_ConvertUTF16toUTF8(aGroup).get(), NS_ConvertUTF16toUTF8(aMethod).get()));
  MOZ_ASSERT(!aRv.Failed());
  if (mDisabled) {
    aRv.ThrowInvalidStateError("Agent is disabled");
    return nullptr;
  }
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(aCx, "BerytusAgentTarget"_ns, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }

  // MOZ_ASSERT(actor->GetWrapper());
  JS::Rooted<JSObject*> actorJsImpl(aCx, actor->GetWrapper());
  JSAutoRealm ar(aCx, actorJsImpl);

  JS::Rooted<JS::Value> sendQuery(aCx);
  if (NS_WARN_IF(!JS_GetProperty(aCx, actorJsImpl, "sendQuery", &sendQuery))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (NS_WARN_IF(!sendQuery.isObject())) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (NS_WARN_IF(!JS::IsCallable(&sendQuery.toObject()))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  JS::Rooted<JS::Value> msgName(aCx, JS::StringValue(JS_NewUCStringCopyZ(aCx, u"BerytusAgentTarget:invokeRequestHandler")));
  JS::Rooted<JS::Value> promiseVal(aCx);
  JS::RootedVector<JS::Value> args(aCx);
  if (NS_WARN_IF(!args.append(msgName))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  JS::Rooted<JSObject*> msgData(aCx, JS_NewPlainObject(aCx));
  if (NS_WARN_IF(!msgData)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  JS::Rooted<JS::Value> managerId(aCx, JS::StringValue(JS_NewUCStringCopyN(aCx, mManagerId.get(), mManagerId.Length())));
  if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "managerId", managerId))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  const char16_t* groupBuf;
  aGroup.GetData(&groupBuf);
  JS::Rooted<JS::Value> group(
    aCx, JS::StringValue(JS_NewUCStringCopyN(aCx, groupBuf, aGroup.Length())));
  if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "group", group))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  const char16_t* methodBuf;
  aMethod.GetData(&methodBuf);
  JS::Rooted<JS::Value> method(
    aCx, JS::StringValue(JS_NewUCStringCopyN(aCx, methodBuf, aMethod.Length())));
  if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "method", method))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  JS::Rooted<JS::Value> reqCxJS(aCx);
  if (NS_WARN_IF(!ToJSVal(aCx, aReqCx, &reqCxJS))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "requestContext", reqCxJS))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (!aReqArgsJs.isUndefined()) {
    JS::Rooted<JS::Value> wrappedReqArgs(aCx, aReqArgsJs);
    if (NS_WARN_IF(!JS_WrapValue(aCx, &wrappedReqArgs))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
    if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "requestArgs", wrappedReqArgs))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
  }
  if (NS_WARN_IF(!args.append(JS::ObjectValue(*msgData)))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (!JS_CallFunctionValue(aCx, actorJsImpl, sendQuery, JS::HandleValueArray(args), //JS::HandleValueArray::empty(), //JS::HandleValueArray(aData),
                &promiseVal)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (NS_WARN_IF(!promiseVal.isObject())) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<dom::Promise> promise = dom::Promise::Create(mGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  promise->MaybeResolve(promiseVal);
  return promise.forget();
}

NS_IMPL_ADDREF_INHERITED(Owned${AgentProxyGenerator.className}, ${AgentProxyGenerator.className})
NS_IMPL_RELEASE_INHERITED(Owned${AgentProxyGenerator.className}, ${AgentProxyGenerator.className})
NS_INTERFACE_MAP_BEGIN(Owned${AgentProxyGenerator.className})
NS_INTERFACE_MAP_END_INHERITING(${AgentProxyGenerator.className})

Owned${AgentProxyGenerator.className}::Owned${AgentProxyGenerator.className}(
    nsIGlobalObject* aGlobal, const nsAString& aManagerId)
    : ${AgentProxyGenerator.className}(aGlobal, aManagerId) {}

Owned${AgentProxyGenerator.className}::~Owned${AgentProxyGenerator.className}() {}

void Owned${AgentProxyGenerator.className}::Disable() {
  mDisabled = true;
}

${this.defs.filter(d => !(d instanceof MethodDef)).map(d => d.implementation).join("\n")}

${this.defs.filter(d => d instanceof MethodDef).map((m) => `${m.implementation}`).join("\n")}

}  // namespace mozilla::berytus`;
    }

    generateHeader() {
        return `${license}
#ifndef BERYTUS_${AgentProxyGenerator.className.toUpperCase()}_H_
#define BERYTUS_${AgentProxyGenerator.className.toUpperCase()}_H_

#include "nsISupports.h"
#include "nsCycleCollectionParticipant.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/TypedArray.h" // ArrayBuffer
#include "mozilla/Variant.h"
#include "mozilla/dom/DOMException.h" // for Failure's Exception
#include "mozilla/Logging.h"
#include "mozilla/dom/Record.h"
#include "mozilla/dom/PromiseNativeHandler.h"

using mozilla::LogLevel;

namespace mozilla::berytus {

using ArrayBuffer = mozilla::dom::ArrayBuffer;
using ArrayBufferView = mozilla::dom::ArrayBufferView;
template <typename K, typename V>
using Record = mozilla::dom::Record<K, V>;

template <typename... T>
class SafeVariant {
public:
  SafeVariant() = delete;
};

class StaticStringBase {
public:
  virtual const nsLiteralString& GetString() const = 0;
  operator nsLiteralString const&() const {
    return GetString();
  }
protected:
  virtual ~StaticStringBase() {}
};

template <typename T>
bool JSValIs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  static_assert(false, "No JSValIs specialisation was found!");
  return false;
}

template <typename T>
bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, T& aRv) {
  static_assert(false, "No FromJSVal specialisation was found!");
  return false;
}

template <typename T>
bool ToJSVal(JSContext* aCx, const T& aValue, JS::MutableHandle<JS::Value> aRv) {
  static_assert(false, "No ToJSVal specialisation was found!");
  return false;
}

${this.defs.filter(d => !(d instanceof MethodDef)).map(def => def.definition).join("\n")}

class ${AgentProxyGenerator.className} : public nsISupports {
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_CLASS(${AgentProxyGenerator.className})

public:
  ${AgentProxyGenerator.className}(nsIGlobalObject* aGlobal, const nsAString& aManagerId);
  bool IsDisabled() const;

  template <typename W1, typename W2>
  already_AddRefed<dom::Promise> CallSendQuery(JSContext *aCx,
                                               const nsAString & aGroup,
                                               const nsAString &aMethod,
                                               const W1& aReqCx,
                                               const W2* aReqArgs,
                                               ErrorResult& aRv);
  template <typename W1>
  already_AddRefed<dom::Promise> CallSendQuery(JSContext *aCx,
                                               const nsAString & aGroup,
                                               const nsAString &aMethod,
                                               const W1& aReqCx,
                                               JS::Handle<JS::Value> aReqArgsJs,
                                               ErrorResult& aRv);

protected:
  virtual ~${AgentProxyGenerator.className}();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  nsString mManagerId;
  bool mDisabled;

public:
${this.defs.filter(d => d instanceof MethodDef).map(def => def.definition).join("\n").replace(/^(.*)$/gm, "  $1")}

};

class Owned${AgentProxyGenerator.className} final : public ${AgentProxyGenerator.className} {
public:
    NS_DECL_ISUPPORTS_INHERITED

    Owned${AgentProxyGenerator.className}(nsIGlobalObject* aGlobal, const nsAString& aManagerId);

    void Disable();
protected:
    ~Owned${AgentProxyGenerator.className}();
};

// based on dom::MozPromiseRejectOnDestruction in PromiseNativeHandler.h
// we just reject with a berytus::Failure instead of an nsresult.
template <typename T>
class MozPromiseRejectWithBerytusFailureOnDestruction final
    : public dom::MozPromiseRejectOnDestructionBase {
 public:
  MozPromiseRejectWithBerytusFailureOnDestruction(const RefPtr<T>& aMozPromise,
                                StaticString aCallSite)
      : mMozPromise(aMozPromise), mCallSite(aCallSite) {
    MOZ_ASSERT(aMozPromise);
  }

 protected:
  ~MozPromiseRejectWithBerytusFailureOnDestruction() override {
    // Rejecting will be no-op if the promise is already settled
    mMozPromise->Reject(berytus::Failure(NS_BINDING_ABORTED), mCallSite);
  }

  RefPtr<T> mMozPromise;
  StaticString mCallSite;
};

}  // namespace mozilla::berytus

#endif`;
    }
}

export const generateDomProxy = async () => {
    const h = new RequestHandlerParser();
    const groups = h.getGroups();
    const generator = new AgentProxyGenerator();

    const typeIterator = h.typeIterator();
    let item = typeIterator.next();
    let parameters: Array<TypedMember> = [];
    while (! item.done) {
        const {
            parsedType,
            source,
            method,
            group,
            paramName
        } = item.value;
        const type = generator.defineType(parsedType);
        if (source === "parameter") {
            const typedMember = new ArgumentMember(
                type,
                new MemberSymbol(paramName)
            );
            parameters.push(typedMember);
        }
        if (source === "returnType") {
            generator.addMethod(
                group,
                method.name,
                parameters,
                type
            );
            parameters.splice(0, parameters.length);
        }
        item = typeIterator.next();
    }

    return {
        header: generator.generateHeader(),
        impl: generator.generateImpl()
    };
}