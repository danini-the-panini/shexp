## Code Generation and Factoring for Fast Evaluation 
## of Low-order Spherical Harmonic Products and Squares 

## John Snyder, 2006

REAL = "double"
INDENT = "  "

# The code generator is given n as input, as well as a
# list of the nonzero entries in Γ, stored as an array
# of index triples (i,j,k), and their corresponding
# coefficient Γijk
def code_gen(n,gamma)

  # The algorithm makes use of a 2D array N[i][j]
  # (N[i][j]=Nij), which stores the current number of
  # different k values in  nonzero triples (i,j,k).
  # It is initialized to 0 for all (i,j).
  nn = (0...n*n).map { (0...n*n).map { 0 } }


  # Then the entries in T are visited and used to
  # increment entries in N, depending on the number
  # of unique indices in the triple
  # (T[p].i, T[p].j, T[p].k)
  total_entries = 0
  gamma.each do |triple, c|
    i, j, k = triple

    # If there is only one unique index i in this
    # triple, we increment N[i][i].
    if i == j && j == k
      nn[i][i]+=1

      total_entries+=1
      # puts "ONE: #{i} #{j} #{k}"
      # puts "N[#{i}][#{i}] := #{nn[i][j]}"
      # puts

    # If there are three, we increment N[ i ][ j ],
    # N[ i ][ k ], and N[ j ][ k ], corresponding to
    # all different pairs.
    elsif i != j && j != k
      nn[i][j]+=1
      nn[i][k]+=1
      nn[j][k]+=1

      total_entries+=3
      # puts "THREE: #{i} #{j} #{k}"
      # puts "N[#{i}][#{j}] := #{nn[i][j]}"
      # puts "N[#{i}][#{k}] := #{nn[i][k]}"
      # puts "N[#{j}][#{k}] := #{nn[j][k]}"
      # puts

    # If there are two, we increment the entry
    # corresponding to the single repeated index i,
    # N[i][i].
    elsif i == j || i == k
      nn[i][i]+=1

      total_entries+=1
      # puts "TWO: #{i} #{j} #{k}"
      # puts "N[#{i}][#{i}] := #{nn[i][j]}"
      # puts
    elsif j == k
      nn[j][j]+=1

      total_entries+=1
      # puts "TWO: #{i} #{j} #{k}"
      # puts "N[#{j}][#{j}] := #{nn[j][j]}"
      # puts
    end
  end

  # puts
  # nn.each { |x| p x }
  # p total_entries
  # puts

  nt = []
  while total_entries>0 do
    # Then we can search for the index pair ( i , j )
    # having the greatest N[ i ][ j ].
    nm, im, jm = 0, 0, 0
    (0...n*n).each do |i|
      (0...n*n).each do |j|
        if nn[i][j] > nm
          nm = nn[i][j]
          im, jm = i, j
        end
      end
    end

    # puts "#{nm} #{im} #{jm}"
    # puts

    # We record this index pair, its corresponding
    # N[ i ][ j ] ...
    record = [im, jm, nm]

    # ... and the list (having N[ i ][ j ]
    # elements) of k indices completing the tensor
    # triples containing ( i , j ).
    kk = []
    d = []
    gamma.each do |triple, c|
      i, j, k = triple

      k2 = nil
      if im == i && jm == j
        k2 = k
      end
      if im == i && jm == k
        k2 = j
      end
      if im == j && jm == k
        k2 = i
      end

      unless k2.nil? || kk.include?(k2)

        kk << k2
        d << c

        # Then we update the N array by reducing counts
        # in all relevant entries corresponding to the
        # triples ( i , j , k ) we just processed ...
        if i == j && j == k
          nn[i][i]-=1

          total_entries-=1

        elsif i != j && j != k
          nn[i][j]-=1
          nn[i][k]-=1
          nn[j][k]-=1

          total_entries-=3

        elsif i == j || i == k
          nn[i][i]-=1
          total_entries-=1
        elsif j == k then
          nn[j][j]-=1
          total_entries-=1
        end
      end
    end
    nt << (record << kk << d)

    # p record
    # puts
    # nn.each { |x| p x }
    # p total_entries
    # puts


    # ... and search for the next best index pair,
    # until all tensor components have been accounted for
  end


  multiplies = {:pr => 0, :sq => 0, :mat => 0}
  additions = {:pr => 0, :sq => 0, :mat => 0}

  # keeping track of when a vector component of the
  # output, c[ i ], is first assigned and using a
  # simple assignment “=” in that initial case,
  # followed by “+=” for later assignments.
  assigned = {:pr => Array.new(n*n,false), :sq => Array.new(n*n,false) }
  assign_c = -> i,f {
    unless assigned[f][i]
      assigned[f][i] = true
      "c[#{i}] = "
    else
      # additions[f] += 1
      "c[#{i}] += "
    end
  }

  # ta = d1 *a[ k1 ] + d2 *a[ k2 ] + ... + dN *a[ kN ];
  product_add = -> a,dk {
    dk.map { |dm,km|
      "#{dm}*#{a}[#{km}]"
    }.reduce { |x,y|
      "#{x} + #{y}"
    }
  }

  # ta = ( 2*d1 )*a[ k1 ] + ( 2*d2 )*a[ k2 ] + ... + ( 2*dN )*a[ kN ];
  square_add = -> dk {
    dk.map { |dm,km|
      "#{2*dm}*a[#{km}]"
    }.reduce { |x,y|
      "#{x} + #{y}"
    }
  }

  code = {:pr => [], :sq => []}

  code[:pr] << "#{REAL} ta, tb, t" << ""
  code[:sq] << "#{REAL} ta, t" << ""

  # The code is then generated in ( i , j ) order of
  # these index pairs
  nt.sort.each do |i, j, nm, k, d|

    comment = "// [#{i},#{j}]: " + k.reduce { |x,y| "#{x}, #{y}" }.to_s

    dk = d.zip(k)
    dk_2 = dk.select { |dm,km| km != i } # km ≠ i

    ## Generate product code
    code[:pr] << comment

    # Case i ≠ j ; assumes km ≠ i , km ≠ j
    if i != j
      # ta = d1 *a[ k1 ] + d2 *a[ k2 ] + ... + dN *a[ kN ];
      code[:pr] << "ta = " + product_add.('a',dk)
      # tb = d1 *b[ k1 ] + d2 *b[ k2 ] + ... + dN *b[ kN ];
      code[:pr] << "tb = " + product_add.('b',dk)
      # c[ i ] += ta*b[ j ] + tb*a[ j ];
      code[:pr] << assign_c[i,:pr] + "ta*b[#{j}] + tb*a[#{j}]"
      # c[ j ] += ta*b[ i ] + tb*a[ i ];
      code[:pr] << assign_c[j,:pr] + "ta*b[#{i}] + tb*a[#{i}]"
      # t = a[ i ]*b[ j ] + a[ j ]*b[ i ];
      code[:pr] << "t = a[#{i}]*b[#{j}] + a[#{j}]*b[#{i}]"
      # c[ k1 ] += d1 *t;
      # c[ k2 ] += d2 *t;
      # ...
      # c[ kN ] += dN *t;
      dk.each do |dm,km|
        code[:pr] << assign_c[km,:pr] + "#{dm}*t"
      end
      # // 3N+6 multiplies, 3N+3 additions
      multiplies[:pr] += 3 * k.size + 6
      additions[:pr] += 3 * k.size + 3

    # Case i = j
    else
      if dk_2.empty?
        code[:pr] << assign_c[i,:pr] + "#{d[0]}*b[#{i}]*a[#{i}]"
        multiplies[:pr] += 2
      else
        # ta = d1 *a[ k1 ] + d2 *a[ k2 ] + ... + dN *a[ kN ]; // km ≠ i
        code[:pr] << "ta = " + product_add.('a',dk_2)
        # tb = d1 *b[ k1 ] + d2 *b[ k2 ] + ... + dN *b[ kN ]; // km ≠ i
        code[:pr] << "tb = " + product_add.('b',dk_2)
        # c[ i ] += ta*b[ i ] + tb*a[ i ];
        code[:pr] << assign_c[i,:pr] + "ta*b[#{i}] + tb*a[#{i}]"
        # t = a[ i ]*b[ i ];
        code[:pr] << "t = a[#{i}]*b[#{i}]"
        # c[ k1 ] += d1 *t;
        # c[ k2 ] += d2 *t;
        # ...
        # c[ kN ] += dN *t;
        dk.each do |dm,km|
          code[:pr] << assign_c[km,:pr] + "#{dm}*t"
        end
        # 3N+1 multiplies, 3N-2 additions
        multiplies[:pr] += 3 * k.size + 1
        additions[:pr] += 3 * k.size - 2
      end
    end

    code[:pr] << ""

    ## Generate square code
    code[:sq] << comment

    # Case i ≠ j ; assumes km ≠ i , km ≠ j
    if i != j
      # ta = ( 2*d1 )*a[ k1 ] + ( 2*d2 )*a[ k2 ] + ... + ( 2*dN )*a[ kN ];
      code[:sq] << "ta = " + square_add.(dk)
      # c[ i ] += ta*a[ j ];
      code[:sq] << assign_c[i,:sq] + "ta*a[#{j}]"
      # c[ j ] += ta*a[ i ];
      code[:sq] << assign_c[j,:sq] + "ta*a[#{i}]"
      # t = a[ i ]*a[ j ];
      code[:sq] << "t = a[#{i}]*a[#{j}]"
      # c[ k1 ] += ( 2*d1 )*t;
      # c[ k2 ] += ( 2*d2 )*t;
      # ...
      # c[ kN ] += ( 2*dN )*t;
      dk.each do |dm,km|
        code[:sq] << assign_c[km,:sq] + "#{2*dm}*t"
      end
      # 2N+3 multiplies, 2N+1 additions
      multiplies[:sq] += 2 * k.size + 3
      additions[:sq] += 2 * k.size + 1

      # Case i = j
    else
      if dk_2.empty?
        code[:sq] << assign_c[i,:sq] + "#{d[0]}*a[#{i}]*a[#{i}]"
        multiplies[:sq] += 2
      else
        # ta = ( 2*d1 )*a[ k1 ] + ( 2*d2 )*a[ k2 ] + ... + ( 2*dN )*a[ kN ]; // k m ≠ i
        code[:sq] << "ta = " + square_add.(dk_2).to_s
        # c[ i ] += ta*a[ i ];
        code[:sq] << assign_c[i,:sq] + "ta*a[#{i}]"
        # t = a[ i ]*a[ i ];
        code[:sq] << "t = a[#{i}]*a[#{i}]"
        # c[ k 1 ] += d1 *t;
        # c[ k 2 ] += d2 *t;
        # ...
        # c[ k N ] += dN *t;
        dk.each do |dm,km|
          code[:sq] << assign_c[km,:sq] + "#{dm}*t"
        end
        # 2N+2 multiplies, 2N additions
        multiplies[:sq] += 2 * k.size + 2
        additions[:sq] += 2 * k.size
      end
    end

    code[:sq] << ""

    ## TODO: generate matrix code
  end

  [:pr,:sq].each do |f|
    code[f] << "// entry count: #{nt.size}"
    code[f] << "// multiply count: #{multiplies[f]}"
    code[f] << "// addition count: #{additions[f]}"
  end

  concat_code = -> f {
    code[f].map { |x| (x.nil? || x.empty?) ? "\n" : "  #{x}\n" }.reduce{ |x,y| x+y }
  }

  product_code = "SH_product_#{n}(const #{REAL} *a, const #{REAL} *b, #{REAL} &*c)\n{\n" + concat_code[:pr] + "}\n"

  square_code = "SH_square_#{n}(const #{REAL} *a, #{REAL} *c)\n{\n" + concat_code[:sq] + "}\n"

  product_code + "\n" + square_code
end

# def sort_triples(gamma)

#   gamma.map do |i,j,k,c|
#     [i,j,k].sort.push(c)
#   end.uniq.sort

# end

N = ARGV[0].to_i
if ARGV.size > 1
  ARGV.replace(ARGV[1])
else
  ARGV.replace([])
end

gamma = eval(ARGF.read)

# gamma.each { |t| p t }


puts code_gen(N, gamma)