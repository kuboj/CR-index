#!/usr/bin/env ruby

f_in = File.open("../genomes/ecoli.fastq", "r")
f_out = File.open("../genomes/ecoli1M.fastq", "w+")
c = 0
num = 1000000
r = -1

f_in.lazy.each_slice(4).with_index do |(_, l, _, _), i|
  r = rand(4)
  next unless r == 0
  f_out << "@#{c}\n" << l.strip << "\n+\n" << ("~" * l.strip.length) << "\n"
  c += 1
  break if c >= num
end
