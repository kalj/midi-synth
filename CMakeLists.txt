cmake_minimum_required(VERSION 3.15)

project(synth C)

add_library(synth STATIC synth.c oscillator.c envelope.c filter.c)
target_link_libraries(synth m)

add_executable(midi_synth main.c)
target_link_libraries(midi_synth synth asound)

add_executable(wavegen wavegen.c)
target_link_libraries(wavegen synth)
