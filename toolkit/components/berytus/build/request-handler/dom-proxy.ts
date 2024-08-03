/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 import { RequestHandlerParser } from "./RequestHandlerParser.js";
import { ParsedType } from "./type-parser.js";

const license = `/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
`;

const capitalise = (str: string) => {
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

interface IType extends InstanceType<typeof TypeSymbol>, IWrappableJSValType, IDef {}

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
        const functionName = 'JSValIsInt32';
        const funcDef = `bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
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
        const functionName = 'Int32FromJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, int32_t& aRv)`;
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
        const functionName = 'Int32ToJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, const int32_t& aValue, JS::MutableHandle<JS::Value> aRv)`;
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

class VoidType extends TypeSymbol implements IType {
    constructor() {
        super('void');
    }

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

class EnumType extends BasicType implements IType {
    members: Array<string>;
    constructor(enumSymbol: string, members: Array<string>) {
        super(enumSymbol);
        this.members = members;
    }

    get definition(): string {
        return `enum ${this.symbol} {
  ${this.members.map(m => `${this.symbol}_${m}`).join(",\n    ")},
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
        const functionName = 'JSValIs' + this.symbol;
        const funcDef = `bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
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
        const functionName = `${this.symbol}FromJSVal`;
        const funcDef = `bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.atArgument()} aRv)`;
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
        const functionName = `${this.symbol}ToJSVal`;
        const funcDef = `bool ${functionName}(JSContext* aCx, const ${this.atArgument()} aValue, JS::MutableHandle<JS::Value> aRv)`;
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

class CallbackType extends TypeSymbol implements IType {
    parameters: Array<IType>;

    constructor(parameters: Array<IType>) {
        super(`std::function<void(${parameters.map(p => p.atArgument()).join(', ')})>`);
        this.parameters = parameters;
    }

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
        const functionName = 'JSValIsString';
        const funcDef = `bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
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
        const functionName = 'StringFromJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, nsString& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  JSString* str = aValue.toString();
  JS::AutoCheckCannotGC nogc;
  size_t len;
  const char16_t* buf =
    JS_GetTwoByteStringCharsAndLength(aCx, nogc, str, &len);
  aRv = nsString(buf, len);
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = 'StringToJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, const nsString& aValue, JS::MutableHandle<JS::Value> aRv)`;
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

class BoolType extends BasicType implements IType {

    constructor() {
        super('bool');
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

    isJsValValidFunction(): GeneratedFunction {
        const functionName = 'JSValIsBool';
        const funcDef = `bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
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
        const functionName = 'BoolFromJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv)`;
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
        const functionName = 'BoolToJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, const bool& aValue, JS::MutableHandle<JS::Value> aRv)`;
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

class MaybeType extends TypeSymbol implements IType {
    subType: IType;

    constructor(subType: IType) {
        if (subType instanceof VariantType) {
            throw new Error(
                "Maybe<T> should not wrap Variant<U>. "
                + "Add NothingType to the Variant subtype instead."
            );
        }
        super(`Maybe<${subType.symbol}>`);
        this.subType = subType;
    }

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
        const functionName = 'JSValIs' + this.symbol.replace(/<|>|,| /g, '_');
        const funcDef = `bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
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
        const functionName = this.symbol.replace(/<|>|,| /g, '_') + 'FromJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, const JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  ${this.subType instanceof VariantType
    ? `
  ${this.subType.symbol}* subRv;
  if (NS_WARN_IF(!${this.subType.importFromJsValFunction().functionName}(aCx, aValue, &subRv))) {
    return false;
  }
  aRv.emplace(*subRv);`
    : `
  aRv.emplace();
  if (NS_WARN_IF(!${this.subType.importFromJsValFunction().functionName}(aCx, aValue, *aRv))) {
    return false;
  }`
}
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = this.symbol.replace(/<|>|,| /g, '_')  + 'ToJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
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
        const functionName = 'JSValIsNothing';
        const funcDef = `bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
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
        const functionName = 'NothingFromJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, Nothing& aRv)`;
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
        const functionName = 'NothingToJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, const Nothing& aValue, JS::MutableHandle<JS::Value> aRv)`;
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

class VariantType extends TypeSymbol implements IType {
    subTypes: Array<IType>;

    constructor(subTypes: Array<IType>) {
        super("Variant<" + subTypes.map(p => p.symbol).join(', ') + ">");
        this.subTypes = subTypes;
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
        return `${this.symbol}&`;
    }
    atReturn(): string {
        return `${this.symbol}`;
    }
    atDefinition(): string {
        return `${this.symbol}`;
    }
    atStruct(): string {
        return `${this.symbol}*`;
    }

    isJsValValidFunction(): GeneratedFunction {
        const functionName = 'JSValIs' + this.symbol.replace(/<|>|,| /g, '_');
        const funcDef = `bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  bool isValid = false;
  ${this.subTypes.map(st => `
  if (NS_WARN_IF(!${st.isJsValValidFunction().functionName}(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    aRv = true;
    return true;
  }
`).join("\n")}
  aRv = false;
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = this.symbol.replace(/<|>|,| /g, '_') + `FromJSVal`;
        const funcDef = `bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.symbol}** aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  bool isValid = false;
  ${this.subTypes.map(st =>
  `if (NS_WARN_IF(!${st.isJsValValidFunction().functionName}(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    ${st instanceof ArrayBufferType ?
    `*aRv = new ${this.symbol}(ArrayBuffer());
    if (NS_WARN_IF(!${st.importFromJsValFunction().functionName}(aCx, aValue, (*aRv)->as<ArrayBuffer>()))) {
      return false;
    }
    return true;
    ` :
    `${st.atDefinition()} nv;
    if (NS_WARN_IF(!${st.importFromJsValFunction().functionName}(aCx, aValue, nv))) {
      return false;
    }
    *aRv = new ${this.symbol}(nv);
    return true;`}
  }`).join("\n")}

  NS_WARN_IF(true);
  return false;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = this.symbol.replace(/<|>|,| /g, '_') + `ToJSVal`;
        const matcherName = `${functionName}Matcher`;
        const funcDef = `bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `struct ${matcherName} {
  JSContext* mCx;
  JS::MutableHandle<JS::Value> mRv;
  ${matcherName}(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
${this.subTypes.map(st => `
  bool operator()(const ${st.atArgument()} aVal) {
    return ${st.exportToJsValFunction().functionName}(mCx, aVal, mRv);
  }`).join("\n")}
};
${funcDef} {
  return aValue.match(${matcherName}(aCx, aRv));
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
        const functionName = 'JSValIs' + this.symbol.replace(/<|>|,| /g, '_');
        const funcDef = `bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
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
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = this.symbol.replace(/<|>|,| /g, '_') + 'FromJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, const JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
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
    if (NS_WARN_IF(!${this.subType.importFromJsValFunction().functionName}(aCx, value, item))) {
      return false;
    }
    aRv.AppendElement(item);
  }
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = this.symbol.replace(/<|>|,| /g, '_') + 'ToJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const ${this.subType.symbol}& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!${this.subType.exportToJsValFunction().functionName}(aCx, item, &value))) {
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
        const functionName = 'JSValIsArrayBuffer';
        const funcDef = `bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (!aValue.isObject()) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  aRv = JS::IsArrayBufferObject(obj);
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = 'ArrayBufferFromJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
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
  if (NS_WARN_IF(!JS::GetObjectAsArrayBuffer(obj, &length, &bytes))) {
    return false;
  }
  if (NS_WARN_IF(aRv.inited())) {
    return false;
  }
  if (NS_WARN_IF(!aRv.Init(obj))) {
    return false;
  }
  return true;
}`
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = 'ArrayBufferToJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
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

    get id() {
        return this.symbol;
    }

    get definition() {
        const variantMembers = this.members.filter(({ type }) => type instanceof VariantType);
        const otherMembers = this.members.filter(({ type }) => !(type instanceof VariantType));
        return `struct ${this.symbol} {
  ${otherMembers.map(m => m.toString() + ';').join("\n  ")}
  ${variantMembers.map(m => m.toString() + ' = nullptr').join(";\n  ")};
  ${variantMembers.length > 0 ? `
  ~${this.symbol}() {
    ${variantMembers.map(({ member }) => `delete ${member.atStruct()};`).join("\n")}
  }` : ''}
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
        const functionName = 'JSValIs' + this.symbol;
        const funcDef = `bool ${functionName}(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  ${this.members.map(({ type, member }) => `
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "${member.baseName}", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!${type.isJsValValidFunction().functionName}(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  ` ).join("\n")}
  aRv = true;
  return true;
}`
        }
    }

    importFromJsValFunction(): GeneratedFunction {
        const functionName = this.symbol + 'FromJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, JS::Handle<JS::Value> aValue, ${this.symbol}& aRv)`;
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
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "${member.baseName}", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!${type.importFromJsValFunction().functionName}(aCx, propVal, ${type instanceof VariantType ? '&' : ''}aRv.${member.atStruct()}))) {
    return false;
  }
  `).join("\n")}
  return true;
}
            `
        }
    }

    exportToJsValFunction(): GeneratedFunction {
        const functionName = this.symbol + 'ToJSVal';
        const funcDef = `bool ${functionName}(JSContext* aCx, const ${this.symbol}& aValue, JS::MutableHandle<JS::Value> aRv)`;
        return {
            functionName,
            functionDef: `${funcDef};`,
            functionImpl: `${funcDef} {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  ${this.members.map(({ type, member }, i) => `
  JS::Rooted<JS::Value> memberVal${i}(aCx);
  ${type instanceof VariantType ? `if (NS_WARN_IF(!aValue.${member.atStruct()})) {
    return false;
  }` : ``}
  if (NS_WARN_IF(!${type.exportToJsValFunction().functionName}(aCx, ${type instanceof VariantType ? '*' : ''}aValue.${member.atStruct()}, &memberVal${i}))) {
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

class MethodDef implements IDef {
    name: string;
    group: string;
    method: string;
    parameters: Array<ArgumentMember>;
    returnType: IType;

    constructor(group: string, method: string, parameters: Array<ArgumentMember>, returnType: IType) {
        this.group = group;
        this.method = method;
        this.name = capitalise(this.group) + "_" + capitalise(this.method);
        this.parameters = parameters;
        this.returnType = returnType;
    }

    get id() {
        return this.name;
    }

    get definition() {
        return `${this.returnType.atReturn()} ${this.name}(${this.parameters.map(p => p.toString()).join(', ')});`
    }

    get implementation() {
        return undefined;
    }
}

class UniqueArray<T extends { id: string }> extends Array<T> {
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
        returnType: IType
    ) {
        const cb =  new CallbackType([returnType]);
        const cbName = `ResolvedCallbackType${capitalise(group)}${capitalise(method)}`;
        this.defs.push(
            cb
        );
        this.defs.push(
            new MethodDef(
                group,
                method,
                [...parameters, new ArgumentMember(cb, new MemberSymbol( 'resolvedCb'))],
                new VoidType()
            )
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
                    return this.defineNumber(parsedType);
                case "string":
                    return this.defineString(parsedType);
                case "boolean":
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
        if (
            parsedType.type === "undefined"
            || parsedType.type === "null"
            || parsedType.type === "any"
        ) {
            return new VoidType();
        }
        if (parsedType.type === "ArrayBuffer") {
            return this.defineArrayBuffer(parsedType);
        }
        if (parsedType.type === "boolean") {
            return this.defineBoolean(parsedType);
        }
        if (parsedType.type === "number") {
            return this.defineNumber(parsedType);
        }
        if (
            parsedType.type === "string" ||
            parsedType.type === "pattern"
        ) {
            return this.defineString(parsedType);
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
            && !(
                parsedType.type === "literal"
                && typeof parsedType.value === "string"
            )
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
        const numType = new Int32Type();
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

    defineUnion(parsedType: ParsedType): IType {
        if (parsedType.type !== "union") {
            throw new Error(
                "Wrong type passed to defineUnion"
            );
        }
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
        const variantType = new VariantType(filteredSubTypes);
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
        const enumType = new EnumType(
            parsedType.alias,
            parsedType.choices.map(({ name, value }) =>  `${name} = ${value}`)
        );
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

namespace mozilla::berytus {

NS_IMPL_CYCLE_COLLECTION(${AgentProxyGenerator.className}, mGlobal)
NS_IMPL_CYCLE_COLLECTING_ADDREF(${AgentProxyGenerator.className})
NS_IMPL_CYCLE_COLLECTING_RELEASE(${AgentProxyGenerator.className})
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(${AgentProxyGenerator.className})
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

${AgentProxyGenerator.className}::${AgentProxyGenerator.className}(
    nsIGlobalObject* aGlobal, const nsAString& aManagerId)
    : mGlobal(aGlobal), mManagerId(aManagerId) {}

${AgentProxyGenerator.className}::~${AgentProxyGenerator.className}() {}

${this.defs.filter(d => !(d instanceof MethodDef)).map(d => d.implementation).join("\n")}

${this.defs.filter(d => d instanceof MethodDef).map(({ group, method, parameters, returnType, name, definition }) =>
`${returnType.atReturn()} ${AgentProxyGenerator.className}::${name}(${parameters.map(p => p.toString()).join(', ')}) {
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    return;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    return;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    return;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
      wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    return;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    return;
  }

  JS::Rooted<JSString*> rManagerId(
      cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    return;
  }

  nsString group = u"${group}"_ns;
  nsString method = u"${method}"_ns;

  JS::Rooted<JSString*> rGroup(
      cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    return;
  }

  JS::Rooted<JSString*> jMethod(
      cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    return;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!${parameters[0].type.exportToJsValFunction().functionName}(cx, aContext, &vReqCx))) {
    return;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    return;
  }
  ${! (parameters[1].type instanceof CallbackType) ? `
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!${parameters[1].type.exportToJsValFunction().functionName}(cx, aArgs, &vReqArgs))) {
    return;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    return;
  }
  ` : ``}

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    return;
  }
  return;
}`).join("\n")}

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

namespace mozilla::berytus {

using ArrayBuffer = mozilla::dom::ArrayBuffer;
${this.defs.filter(d => !(d instanceof MethodDef)).map(def => def.definition).join("\n")}

class ${AgentProxyGenerator.className} final : public nsISupports {
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_CLASS(${AgentProxyGenerator.className})

public:
  ${AgentProxyGenerator.className}(nsIGlobalObject* aGlobal, const nsAString& aManagerId);

protected:
  ~${AgentProxyGenerator.className}();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  nsString mManagerId;

public:
${this.defs.filter(d => d instanceof MethodDef).map(def => def.definition).join("\n").replace(/^(.*)$/gm, "  $1")}

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
            // last item in the method, add the
            // callback and method defs

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