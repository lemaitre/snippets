#! /usr/bin/env python

import functools
from enum import Enum

class CharClass(Enum):
  SPACE = ' '
  WORD = 'a'
  SYMBOL = '*'
  NONE = ''

  @staticmethod
  def from_chr(c):
    if c.isalnum() or c in '_$':
      return CharClass.WORD
    if c.isspace():
      return CharClass.SPACE
    return CharClass.SYMBOL


class Token(object):
  def __init__(self, string):
    self.string = str(string)

  def __repr__(self):
    return "<Token {}>".format(repr(self.string))
  def __str__(self):
    return self.string

def tokenize(string):
  acc = []
  last_chr = CharClass.NONE
  
  for c in string:
    cc = CharClass.from_chr(c)
    if cc == CharClass.SYMBOL or cc != last_chr:
      if acc:
        yield Token("".join(acc))
      acc = [c]
    else:
      acc.append(c)
    last_chr = cc
  yield Token("".join(acc))

def untokenize(token_stream):
  return "".join((str(token) for token in token_stream))

class PreprocessStop(Exception):
  def __init__(self, message, token = None):
    super(PreprocessStop, self).__init__(message)
    self.token = token
class PreprocessError(Exception):
  pass


class Macro:
  def __init__(self, name, f):
    self.name = name
    self.f = f
  def __call__(self, *args):
    return self.f(*args)
  def __repr__(self):
    return "<Macro {}: {}>".format(self.name, self.f)

def macro(name = None):
  def decorator(f):
    @functools.wraps(f)
    def wrapper(*args):
      return tokenize(f(*[untokenize(arg).strip() for arg in args]))
    return Macro(name or "$" + f.__name__, wrapper)
  return decorator

def process(token_stream, macros = None, stop_on_comma = False):
  if not macros:
    macros = dict()
  if not isinstance(macros, dict):
    _macros = macros
    macros = dict()
    for macro in _macros:
      macros[macro.name] = macro

  def recurse(stream = token_stream, stop_on_comma = False, remove_macro = None):
    rmacros = macros
    if remove_macro is not None:
      rmacros = dict(rmacros)
      del rmacros[str(remove_macro)]
    return process(stream, rmacros, stop_on_comma)


  token_eaten = True
  token = None
  while True:
    try:
      if token_eaten:
        token = next(token_stream)
      token_eaten = True

      if token.string == '(':
        yield token
        try:
          for t in recurse():
            yield t
        except PreprocessStop as e:
          if e.token.string != ')':
            raise PreprocessError("Unexcpeted '{}' token".format(e.token))
          yield e.token

      elif token.string == ')':
        raise PreprocessStop("Unexpected ')' token", token)
      elif token.string == ',' and stop_on_comma:
        raise PreprocessStop("Unexpected ',' token", token)
      else:
        if token.string in macros:
          macro_name = token.string
          macro = macros[macro_name]
          token = next(token_stream)
          if token.string == '(':
            args = []
            while True:
              arg_tokens = []
              try:
                for arg_token in recurse(stop_on_comma = True):
                  arg_tokens.append(arg_token)
              except PreprocessStop as e:
                if e.token.string == ",":
                  args.append(arg_tokens)
                elif e.token.string == ")":
                  args.append(arg_tokens)
                  break
                else:
                  raise PreprocessError("Unexcpeted '{}' token".format(e.token))
              else:
                raise PreprocessError("No closing ')' has been found to macro call '{}'".format(macro_name))
            for t in recurse(stream = macro(*args), remove_macro = macro_name):
              yield t

          else:
            for t in recurse(stream = macro(), remove_macro = macro_name):
              yield t
            if token.string == ')':
              raise PreprocessStop("Unexpected ')' token", token)
            elif token.string == ',' and stop_on_comma:
              raise PreprocessStop("Unexpected ',' token", token)
            token_eaten = False



        else:
          yield token

    except StopIteration:
      break

    
if __name__ == "__main__":
  class Vec:
    @macro()
    def add(a, b):
      return "{} + {}".format(a, b)

  text = """
  vadd(1+2, 3+4);
  $add(test(12), $add(azerty+1, plop));
  """

  print(text)
  print("======")
  print(untokenize(process(macros = [Vec.add], token_stream = tokenize(text))))
