require_relative 'gen_helper'

N = ARGV[0].to_i

source = []
source << define('N_BANDS', N)
source << define('N_COEFFS', N*N)
source << ''
source << source_line(PRODUCT_SIG)
source << source_line(SQUARE_SIG)
source << source_line(MATRIX_SIG)

puts source_guard(source.join("\n"), 'SH_FUNCTIONS_H')
