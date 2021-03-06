open Import;

type t('a) = BS.Js.t('a);

type type_ =
  | Null
  | Undefined
  | Boolean
  | Number
  | String
  | Array
  | Object
  | Function;

let typeName = BS.Js.typeof;

let type_ = (js: t('a)) : type_ =>
  switch (typeName(js)) {
  | "undefined" => Undefined
  | "boolean" => Boolean
  | "number" => Number
  | "string" => String
  | "function" => Function
  | "object" =>
    if (BS.Js.unsafe_le(RE.Obj.magic(js), 0)) {
      Null;
    } else if (BS.Js.Array.isArray(js)) {
      Array;
    } else {
      Object;
    }
  | _ => failwith("unknown type")
  };

let fromBool = BS.Js.Boolean.to_js_boolean;

let fromList = List.toArray;

let fromOption = BS.Js.Null.fromOption;

let fromOptMap = (opt, ~f) => Option.map(opt, ~f) |> BS.Js.Null.fromOption;

let toBool = BS.Js.to_bool;

let toOption = BS.Js.toOption;

let log = BS.Js.log;

let diet = (js: t('a)) : t('a) => {
  let rec diet0 = (js: t('a)) =>
    switch (type_(js)) {
    | Object =>
      let dict: BS.Js.Dict.t(t('a)) = RE.Obj.magic(js);
      let newDict = BS.Js.Dict.empty();
      RE.Array.iter(
        key => {
          let value = BS.Js.Dict.unsafeGet(dict, key);
          switch (type_(value)) {
          | Null => ()
          | _ => BS.Js.Dict.set(newDict, key, diet0(Obj.magic(value)))
          };
        },
        BS.Js.Dict.keys(dict)
      );
      Obj.magic(newDict);
    | _ => js
    };
  diet0(js);
};

module Array = {
  include BS.Js.Array;
};

module Boolean = {
  include BS.Js.Boolean;
  type t = BS.Js.boolean;
};

module Dict = {
  include BS.Js.Dict;
};

module List = {
  include BS.Js.List;
};

module Null = {
  include BS.Js.Null;
};

module Nullable = {
  include BS.Js.Nullable;
};

module Undefined = {
  include BS.Js.Undefined;
};

module Json = {
  include BS.Js.Json;
};

module Option = {
  include BS.Js.Option;
};

module Result = {
  include BS.Js.Result;
};

module Promise = {
  include BS.Js.Promise;
};

module False = {
  type t('a);
  let from = (opt: option('a)) : t('a) =>
    switch opt {
    | None => RE.Obj.magic(BS.Js.false_)
    | Some(value) => RE.Obj.magic(value)
    };
  let to_ = (opt: t('a)) : option('a) =>
    if (RE.Obj.magic(opt) === BS.Js.false_) {
      None;
    } else {
      Some(RE.Obj.magic(opt));
    };
  let some = value => from(Some(value));
  let none = () => from(None);
};

module Any = {
  type t = BS.Js.t(unit);
  let return = (value: 'a) : t => RE.Obj.magic(value);
  let string = (value: t) : option(string) =>
    switch (type_(value)) {
    | String => Some(RE.Obj.magic(value))
    | _ => None
    };
};

module Enumerator = {
  type t('value);
  type elt('value);
  module Basic = {
    [@bs.send] external next : t('value) => elt('value) = "next";
    [@bs.get] external isDone : elt('value) => Boolean.t = "done";
    [@bs.get] external value : elt('value) => 'value = "value";
  };
  let next = each : option('value) => {
    let elt = Basic.next(each);
    Basic.isDone(elt) |> BS.Js.to_bool ? None : Some(Basic.value(elt));
  };
  let each = (each: t('value), ~f: 'value => unit) : unit => {
    let rec each0 = () =>
      switch (next(each)) {
      | None => ()
      | Some(value) =>
        f(value);
        each0();
      };
    each0();
  };
};