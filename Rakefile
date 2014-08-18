require 'rake/clean'
require 'rplusplus'

CC = 'clang++'

LIBS = ['glew', 'glfw3', 'eigen3']
LIB_CFLAGS = LIBS.map { |lib|
                %x[ pkg-config --cflags #{lib} ].gsub(/\n/,' ')
             }.flatten.join(' ')
LDLIBS = LIBS.map {
           |lib| %x[ pkg-config --static --libs #{lib} ].gsub(/\n/,' ')
         }.flatten.join(' ')

WARNING_FLAGS = '-Wall -Wextra -Weffc++ -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-parameter -Wstrict-overflow=5 -Wfloat-equal -Wshadow -Wc++0x-compat -Wconversion -Wsign-conversion -Wmissing-declarations -Woverloaded-virtual -Wsign-promo -pedantic'
FORMATTING_FLAGS = '-fmessage-length=80 -fdiagnostics-show-option'
CFLAGS = "#{WARNING_FLAGS} #{FORMATTING_FLAGS} #{LIB_CFLAGS} -g -std=c++11 -pipe"

LDFLAGS = "#{LDLIBS} -g"

RPP = RPlusPlus::Environment.new

BUILDS = RPP.builds
OBJECTS = RPP.objects
ERBS = RPP.erbs
ERB_TARGETS = ERBS.keys
ERBS.delete 'sh_functions.cpp'

CLOBBER.include(*OBJECTS.keys, *BUILDS.keys, ERB_TARGETS, 'sh_lut.txt')

SH_BANDS = 3
LUT_SIZE = 128

task :default => 'main'

def build target, objects
  puts "---------- Building: #{target}"
  sh "#{CC} #{objects.join(' ')} -o #{target} #{LDFLAGS}"
end

def compile object, source
  puts "---------- Compiling: #{object}"
  sh "#{CC} #{CFLAGS} -c #{source} -o #{object}"
end

def erb target, source
  puts "---------- Generating: #{target}"
  File.write(target, ERB.new(File.read(source)).result)
end

ERBS.each do |target, sources|
  file target => sources do |t|
    erb target, sources.first
  end
end

file 'sh_functions.cpp' => ['sh_functions.cpp.erb', 'gen_sh_functions.rb', 'gen_coeffs'] do |t|
  puts "---------- Exec: ./gen_coeffs"
  @gamma = eval(`./gen_coeffs #{SH_BANDS}`)
  erb 'sh_functions.cpp', 'sh_functions.cpp.erb'
end

OBJECTS.each do |object, sources|
  file object => sources do |t|
    compile object, sources.first
  end
end

BUILDS.each do |target, objects|
  file target => objects do |t|
    build target, objects
  end
end

file 'sh_lut.txt' => 'sh_lut' do
  sh "./sh_lut #{LUT_SIZE} > sh_lut.txt"
end

