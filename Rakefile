require 'rake/clean'

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

GENERATOR_HELPERS = ['gen_helper.rb']
CODE_GENERATOR    = 'gen_sh_functions.rb'
HEADER_GENERATOR  = 'gen_sh_header.rb'
GENERATED_HEADER  = 'sh_functions.h'
GENERATED_CODE    = 'sh_functions.cpp'

MAIN_TARGET = 'main'
GEN_TARGET  = 'gen_coeffs'

BUILDS = {
  GEN_TARGET  => ['green.o', 'gen_coeffs.o'],
  MAIN_TARGET => ['green.o', 'sh_functions.o', 'sh_lut.o',
                  'gfx_boilerplate.o', 'sphere.o', 'shader.o',
                  'main.o'],
}

OBJECTS = {
  'green.o'           => ['green.cpp',           'green.h'],
  'gen_coeffs.o'      => ['gen_coeffs.cpp',      'green.h'],
  'sh_functions.o'    => ['sh_functions.cpp',    'sh_functions.h'],
  'sh_lut.o'          => ['sh_lut.cpp',          'sh_lut.h', 'green.h',
                                                 'sh_functions.h'],
  'gfx_boilerplate.o' => ['gfx_boilerplate.cpp', 'gfx_boilerplate.h',
                                                 'gfx_include.h'],
  'sphere.o'          => ['sphere.cpp',          'sphere.h', 'gfx_include.h'],
  'shader.o'          => ['shader.cpp',          'shader.h', 'gfx_include.h'],
  'main.o'            => ['main.cpp',            'sh_lut.h', 'sh_functions.h',
                                                 'sphere.h', 'shader.h', 'gfx_include.h'],
}

CLOBBER.include(*OBJECTS.keys,*BUILDS.keys,GENERATED_CODE,GENERATED_HEADER)

SH_BANDS = 3

task :default => MAIN_TARGET

def build target, objects
  sh "#{CC} #{objects.join(' ')} -o #{target} #{LDFLAGS}"
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

file GENERATED_CODE => [GEN_TARGET, CODE_GENERATOR, *GENERATOR_HELPERS] do |t|
  sh "./#{GEN_TARGET} #{SH_BANDS} | ruby #{CODE_GENERATOR} #{SH_BANDS} > #{GENERATED_CODE}"
end

file GENERATED_HEADER => [HEADER_GENERATOR, *GENERATOR_HELPERS] do |t|
  sh "ruby #{HEADER_GENERATOR} #{SH_BANDS} > #{GENERATED_HEADER}"
end
