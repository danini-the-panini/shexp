## Code Generation and Factoring for Fast Evaluation
## of Low-order Spherical Harmonic Products and Squares

## John Snyder, 2006

# The code generator is given n as input, as well as a
# list of the nonzero entries in Γ, stored as an array
# of index triples (i,j,k), and their corresponding
# coefficient Γijk

class SHGenerator

  def initialize n, gamma
    @n = n
    @nsq = n*n
    @gamma = gamma
  end

  def build_array
    # The algorithm makes use of a 2D array N[i][j]
    # (N[i][j]=Nj), which stores the current number of
    # different k values in  nonzero triples (i,j,k).
    # It is initialized to 0 for all (i,j).
    n = (0...@nsq).map { (0...@nsq).map { 0 } }


    # Then the entries in T are visited and used to
    # increment entries in N, depending on the number
    # of unique indices in the triple
    # (T[p].i, T[p].j, T[p].k)
    total_entries = 0
    @gamma.each do |triple, c|
      i, j, k = triple

      # If there is only one unique index i in this
      # triple, we increment N[i][i].
      if i == j && j == k
        n[i][i]+=1

      total_entries+=1
      # puts "ONE: #{i} #{j} #{k}"
      # puts "N[#{i}][#{i}] := #{n[i][j]}"
      # puts

      # If there are three, we increment N[ i ][ j ],
      # N[ i ][ k ], and N[ j ][ k ], corresponding to
      # all different pairs.
      elsif i != j && j != k
        n[i][j]+=1
        n[i][k]+=1
        n[j][k]+=1

        total_entries+=3
        # puts "THREE: #{i} #{j} #{k}"
        # puts "N[#{i}][#{j}] := #{n[i][j]}"
        # puts "N[#{i}][#{k}] := #{n[i][k]}"
        # puts "N[#{j}][#{k}] := #{n[j][k]}"
        # puts

      # If there are two, we increment the entry
      # corresponding to the single repeated index i,
      # N[i][i].
      elsif i == j || i == k
        n[i][i]+=1

        total_entries+=1
        # puts "TWO: #{i} #{j} #{k}"
        # puts "N[#{i}][#{i}] := #{n[i][j]}"
        # puts
      elsif j == k
        n[j][j]+=1

        total_entries+=1
        # puts "TWO: #{i} #{j} #{k}"
        # puts "N[#{j}][#{j}] := #{n[j][j]}"
        # puts
      end
    end

    # puts
    # n.each { |x| p x }
    # p total_entries
    # puts

    nt = []
    while total_entries>0 do
      # Then we can search for the index pair ( i , j )
      # having the greatest N[ i ][ j ].
      nm, im, jm = 0, 0, 0
      (0...@nsq).each do |i|
        (0...@nsq).each do |j|
          if n[i][j] > nm
            nm = n[i][j]
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
      @gamma.each do |triple, c|
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
            n[i][i]-=1

            total_entries-=1

          elsif i != j && j != k
            n[i][j]-=1
            n[i][k]-=1
            n[j][k]-=1

            total_entries-=3

          elsif i == j || i == k
            n[i][i]-=1
            total_entries-=1
          elsif j == k then
            n[j][j]-=1
            total_entries-=1
          end
        end
      end
      nt << (record << kk << d)

      # p record
      # puts
      # n.each { |x| p x }
      # p total_entries
      # puts


      # ... and search for the next best index pair,
      # until all tensor components have been accounted for
    end

    @nt = nt.sort

    @assigned = {:pr => Array.new(@nsq,false), :sq => Array.new(@nsq,false) }
  end

  # The code is then generated in ( i , j ) order of
  # these index pairs

  def product_code
    code = []
    multiplies = 0
    additions = 0
    @nt.each do |i, j, nm, k, d|

      dk = d.zip(k)
      dk_2 = dk.select { |dm,km| km != i } # km ≠ i

      code << comment(i,j,k)

      # Case i ≠ j ; assumes km ≠ i , km ≠ j
      if i != j
        # ta = d1 *a[ k1 ] + d2 *a[ k2 ] + ... + dN *a[ kN ];
        code << "ta = " + product_add('a',dk) + ";"
        # tb = d1 *b[ k1 ] + d2 *b[ k2 ] + ... + dN *b[ kN ];
        code << "tb = " + product_add('b',dk) + ";"
        # c[ i ] += ta*b[ j ] + tb*a[ j ];
        code << assign_c(i,:pr) + "ta*b[#{j}] + tb*a[#{j}];"
        # c[ j ] += ta*b[ i ] + tb*a[ i ];
        code << assign_c(j,:pr) + "ta*b[#{i}] + tb*a[#{i}];"
        # t = a[ i ]*b[ j ] + a[ j ]*b[ i ];
        code << "t = a[#{i}]*b[#{j}] + a[#{j}]*b[#{i}];"
        # c[ k1 ] += d1 *t;
        # c[ k2 ] += d2 *t;
        # ...
        # c[ kN ] += dN *t;
        dk.each do |dm,km|
          code << assign_c(km,:pr) + "#{dm}*t;"
        end
        # // 3N+6 multiplies, 3N+3 additions
        multiplies += 3 * k.size + 6
        additions += 3 * k.size + 3

      # Case i = j
      else
        if dk_2.empty?
          code << assign_c(i,:pr) + "#{d[0]}*b[#{i}]*a[#{i}];"
          multiplies += 2
        else
          # ta = d1 *a[ k1 ] + d2 *a[ k2 ] + ... + dN *a[ kN ]; // km ≠ i
          code << "ta = " + product_add('a',dk_2) + ";"
          # tb = d1 *b[ k1 ] + d2 *b[ k2 ] + ... + dN *b[ kN ]; // km ≠ i
          code << "tb = " + product_add('b',dk_2) + ";"
          # c[ i ] += ta*b[ i ] + tb*a[ i ];
          code << assign_c(i,:pr) + "ta*b[#{i}] + tb*a[#{i}];"
          # t = a[ i ]*b[ i ];
          code << "t = a[#{i}]*b[#{i}];"
          # c[ k1 ] += d1 *t;
          # c[ k2 ] += d2 *t;
          # ...
          # c[ kN ] += dN *t;
          dk.each do |dm,km|
            code << assign_c(km,:pr) + "#{dm}*t;"
          end
          # 3N+1 multiplies, 3N-2 additions
          multiplies += 3 * k.size + 1
          additions += 3 * k.size - 2
        end
      end
    end
    code.join("\n")
  end

  def square_code
    code = []
    multiplies = 0
    additions = 0
    @nt.each do |i, j, nm, k, d|

      dk = d.zip(k)
      dk_2 = dk.select { |dm,km| km != i } # km ≠ i

      code << comment(i,j,k)

      # Case i ≠ j ; assumes km ≠ i , km ≠ j
      if i != j
        # ta = ( 2*d1 )*a[ k1 ] + ( 2*d2 )*a[ k2 ] + ... + ( 2*dN )*a[ kN ];
        code << "ta = " + square_add(dk) + ";"
        # c[ i ] += ta*a[ j ];
        code << assign_c(i,:sq) + "ta*a[#{j}];"
        # c[ j ] += ta*a[ i ];
        code << assign_c(j,:sq) + "ta*a[#{i}];"
        # t = a[ i ]*a[ j ];
        code << "t = a[#{i}]*a[#{j}];"
        # c[ k1 ] += ( 2*d1 )*t;
        # c[ k2 ] += ( 2*d2 )*t;
        # ...
        # c[ kN ] += ( 2*dN )*t;
        dk.each do |dm,km|
          code << assign_c(km,:sq) + "#{2*dm}*t;"
        end
        # 2N+3 multiplies, 2N+1 additions
        multiplies += 2 * k.size + 3
        additions += 2 * k.size + 1

        # Case i = j
      else
        if dk_2.empty?
          code << assign_c(i,:sq) + "#{d[0]}*a[#{i}]*a[#{i}];"
          multiplies += 2
        else
          # ta = ( 2*d1 )*a[ k1 ] + ( 2*d2 )*a[ k2 ] + ... + ( 2*dN )*a[ kN ]; // k m ≠ i
          code << "ta = " + square_add(dk_2).to_s + ";"
          # c[ i ] += ta*a[ i ];
          code << assign_c(i,:sq) + "ta*a[#{i}];"
          # t = a[ i ]*a[ i ];
          code << "t = a[#{i}]*a[#{i}];"
          # c[ k 1 ] += d1 *t;
          # c[ k 2 ] += d2 *t;
          # ...
          # c[ k N ] += dN *t;
          dk.each do |dm,km|
            code << assign_c(km,:sq) + "#{dm}*t;"
          end
          # 2N+2 multiplies, 2N additions
          multiplies += 2 * k.size + 2
          additions += 2 * k.size
        end
      end
    end
    code.join("\n")
  end

  def matrix_code
    mat = {}
    multiplies = 0
    additions = 0
    entries = 0

    (0...@nsq).each do |i|
      (i...@nsq).each do |j|
        mat[[i,j]] = []
      end
    end

    @gamma.each do |triple, c|
      i,j,k = triple

      mat[[i,j]] << matrix_term(k,c)
      mat[[i,k]] << matrix_term(j,c) unless k==j
      mat[[j,k]] << matrix_term(i,c) unless j==i
    end

    code = []

    code << "// compute upper triangular part of matrix"
    mat.each do |index,kc|
      code << "M[#{ij2i(*index)}] = #{matrix_product(kc)};"
      entries += 1 unless kc.empty?
      multiplies += kc.size
      additions += kc.size-1 if kc.size > 1
    end

    code << "// fill in lower triangular part of matrix"
    (0...@nsq).each do |i|
      (0...i).each do |j|
        code << "M[#{i*@nsq+j}] = M[#{j*@nsq+i}]; // #{i},#{j} = #{j},#{i}"
      end
    end

    code.join("\n")
  end

  private

    # keeping track of when a vector component of the
    # output, c[ i ], is first assigned and using a
    # simple assignment “=” in that initial case,
    # followed by “+=” for later assignments.
    def assign_c i, f
      unless @assigned[f][i]
        @assigned[f][i] = true
        "c[#{i}] = "
      else
        # additions[f] += 1
        "c[#{i}] += "
      end
    end

    # ta = d1 *a[ k1 ] + d2 *a[ k2 ] + ... + dN *a[ kN ];
    def product_add a, dk
      dk.map { |dm,km|
        "#{dm}*#{a}[#{km}]"
      }.reduce { |x,y|
        "#{x} + #{y}"
      }
    end

    # ta = ( 2*d1 )*a[ k1 ] + ( 2*d2 )*a[ k2 ] + ... + ( 2*dN )*a[ kN ];
    def square_add  dk
      dk.map { |dm,km|
        "#{2*dm}*a[#{km}]"
      }.reduce { |x,y|
        "#{x} + #{y}"
      }
    end

    def comment i,j,k
      "// [#{i},#{j}]: " + k.reduce { |x,y| "#{x}, #{y}" }.to_s
    end

    def matrix_term k,c
      "#{c}*a[#{k}]"
    end

    def ij2i i,j
      i*@nsq+j
    end

    def matrix_product m
      m.empty? ? "0" : m.reduce { |x,y| "#{x} + #{y}" }
    end

end
