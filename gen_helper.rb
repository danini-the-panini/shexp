
def ifndef macro
  "#ifndef #{macro}\n#{yield}\n\n#endif // #{macro}"
end

def define name, value=''
  "#define #{name} #{value}"
end

def source_guard source, guard
  ifndef guard do
    "#{define guard}\n\n#{source}"
  end
end

def source_line source
  "#{source};\n"
end

REAL = "double"
INDENT = "  "

PRODUCT_SIG = "void SH_product(const #{REAL} *a, const #{REAL} *b, #{REAL} *c)"
SQUARE_SIG = "void SH_square(const #{REAL} *a, #{REAL} *c)"
MATRIX_SIG = "void SH_matrix(const #{REAL} *a, #{REAL} *M)"

