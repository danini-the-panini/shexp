require 'rake/clean'

CC='clang++'

LIBS=['glew', 'glfw3']
LIB_CFLAGS=LIBS.map { |lib| %x[ pkg-config --cflags #{lib} ].gsub(/\n/,' ').strip }.flatten.join(' ')
LDLIBS=LIBS.map { |lib| %x[ pkg-config --static --libs #{lib} ].gsub(/\n/,' ').strip }.flatten.join(' ')

WARNING_FLAGS='-Wall -Wextra -Weffc++ -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-parameter -Wstrict-overflow=5 -Wfloat-equal -Wshadow -Wc++0x-compat -Wconversion -Wsign-conversion -Wmissing-declarations -Woverloaded-virtual -Wsign-promo -pedantic'
FORMATTING_FLAGS='-fmessage-length=80 -fdiagnostics-show-option'
CFLAGS="#{WARNING_FLAGS} #{FORMATTING_FLAGS} #{LIB_CFLAGS} -g -std=c++11 -pipe"

LDFLAGS='-g'

CODE_GENERATOR='gen_sh_functions.rb'
GENERATED_CODE='sh_functions.h'

MAIN_TARGET='main'
GEN_TARGET='gen_coeffs'

BUILDS={MAIN_TARGET => ['green.o', 'main.o'],
  GEN_TARGET => ['green.o', 'gen_coeffs.o']}

OBJECTS={'green.o' => ['green.C','green.h'],
  'main.o' => ['main.C','green.h','sh_functions.h'],
  'gen_coeffs.o' => ['gen_coeffs.C','green.h']}

CLOBBER.include(*OBJECTS.keys,*BUILDS.keys,'sh_functions.h')

SH_BANDS=3

task :default => MAIN_TARGET

def build target, objects
  puts "Building #{target} from #{objects}"
  sh "#{CC} #{LDLIBS} #{LDFLAGS} -o #{target} #{objects.join(' ')}"
end

def compile object, source
  sh "#{CC} #{CFLAGS} -c #{source} -o #{object}"
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

file GENERATED_CODE => [GEN_TARGET, CODE_GENERATOR] do |t|
  sh "./#{GEN_TARGET} #{SH_BANDS} | ruby #{CODE_GENERATOR} #{SH_BANDS} > #{GENERATED_CODE}"
end
