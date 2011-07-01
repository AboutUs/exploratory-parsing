$KCODE = "U"
require 'rubygems'
require 'sinatra'
require 'haml'
require 'fileutils'
require "diffy"

set :port, 8080
enable :sessions

FileUtils.mkdir_p "runs"
FileUtils.mkdir_p "data"

use Rack::Auth::Basic do |username, password|
  [username, password] == ['guest', 'please']
end

helpers do
  include Rack::Utils

  def h(msg)
    msg = escape_html(msg)
    msg = msg.gsub(/\035/, '<strong style="font-size: 18px;">&Dagger;</strong>')
    msg = msg.gsub(/\036/, '<strong style="font-size: 18px;">&dagger;</strong>')
    msg
  end

  def n int
    int.to_s.reverse.gsub(/...(?=.)/,'\&,').reverse
  end

end

def text_blocks_for_offset(run, offset, length)
  files = `ls runs/#{run}/data`
  position = offset
  files.split("\n").each do |filename|
    filesize = File.stat("runs/#{run}/data/#{filename}").size
    if position <= filesize
      return File.open("runs/#{run}/data/#{filename}", 'r') do |file|
        prefix = position < 1000 ? position : 1000
        file.sysseek position - prefix
        {:filename => filename, :position => position, :offset => offset, :prefix => file.read(prefix), :match => file.read(length), :sufix => file.read(1000)}
      end
    else
      position -= filesize
    end
  end
  raise "I expected to find a file within the given offset, but I did not. (#{[run, offset, length].inspect})"
end

get %r{/runs/([\d]+)\.svg} do |run|
  `cd runs/#{run} && perl ../../fmt.pl tally.dot | dot -Tsvg -otally.svg 2>>output.log`
  send_file "runs/#{run}/tally.svg",
    :type => "image/svg+xml",
    :disposition => 'inline'
end

get %r{/runs/([\d]+)\.dot} do |run|
  content_type "application/graphviz"
  `cd runs/#{run} && perl ../../fmt.pl tally.dot`
end

get %r{/runs/([\d]+)/bindings} do |run|
  send_file "runs/#{run}/bindings.txt",
    :filename => "#{run}.txt",
    :type => "text/plain"
end

get %r{/runs/([\d]+)$} do |run|
  @run = run
  @log = File.read("runs/#{run}/output.log")
  @grammar = File.read("runs/#{run}/parse.leg")
  @pid = File.exist?("runs/#{run}/pid.txt") && File.read("runs/#{run}/pid.txt")
  content_type 'text/html', :charset => 'utf-8'
  haml :run
end

post %r{/runs/([\d]+)/kill} do |run|
  @pid = File.exist?("runs/#{run}/pid.txt") && File.read("runs/#{run}/pid.txt")
  if @pid
    system("kill #{@pid.strip}")
    sleep 0.5
  end
  redirect "/runs/#{run}"
end

post %r{/runs/([\d]+)/gdb} do |run|
  @pid = File.exist?("runs/#{run}/pid.txt") && File.read("runs/#{run}/pid.txt")
  if @pid
    system("cd runs/#{run}; gdb -batch -x ../../gdb.txt parse #{@pid.strip} >>output.log")
  end
  redirect "/runs/#{run}"
end

post %r{/runs/([\d]+)/delete} do |run|
  `rm -r runs/#{run}`
end

get %r{/runs/([\d]+)/offsets} do |run|
  offsets = params[:offsets].split(',')
  @text_blocks = offsets.map do |offset_length_pair|
    offset, length = offset_length_pair.split('-')
    text_blocks_for_offset run, offset.to_i, length.to_i
  end
  content_type 'text/html', :charset => 'utf-8'
  haml :offsets
end

get %r{/runs/([\d]+)/profile} do |run|
  File.read("runs/#{run}/profile.txt")
end

get %r{/runs/([\d]+)/leg} do |run|
  session[:run] = run
  File.read "runs/#{run}/parse.leg"
end

get %r{/runs/([\d]+)/diff} do |run|
  grammars = [run, session[:run]||run].sort.map {|r| "runs/#{r}/parse.leg"}
  grammars << ({ :source => "files" })
  Diffy::Diff.new(*grammars).to_s(:html)
end

get '/' do
  @runs = Dir.chdir('runs'){ Dir['*'] }.sort
  run = session[:run] || @runs.last || nil
  @current_grammar = begin File.read("runs/#{run}/parse.leg") rescue "" end
  session[:vers] = "/"
  haml :index
end

get %r{/vers/(\w+)} do |prefix|
  @runs = Dir.chdir('runs'){ Dir['*'] }.sort
  leg = ""
  @runs = @runs.select do |run|
    leg = begin File.read "runs/#{run}/parse.leg" rescue "" end
    vers = $1 if leg =~ /(\w+)/
    vers =~ /^#{prefix}/
  end
  session[:vers] = "/vers/#{prefix}"
  run = session[:run] = @runs.last || nil
  @current_grammar = begin File.read("runs/#{run}/parse.leg") rescue "" end
  haml :index
end

post '/start' do
  run = session[:run] = Time.new.strftime("%Y%m%d%H%M%S")
  data = session[:data] = params[:data]
  FileUtils.mkdir_p "runs/#{run}/data"
  Dir["data/#{data}/*"].each do |file|
    FileUtils.ln file, "runs/#{run}/data"
  end
  File.open("runs/#{run}/parse.leg", 'w') do |f|
    f.puts params[:grammar]
  end
  unless data =~ /-Selected$/
    FileUtils.mkdir_p "data/#{data}-Selected"
    `cd data/#{data}-Selected; rm selected.txt; touch selected.txt`
    FileUtils.ln "data/#{data}-Selected/selected.txt", "runs/#{run}/"
  end
  system("sh start_run.sh #{run} > runs/#{run}/output.log 2>&1 &")
  redirect session[:vers] || '/'
end

get '/ls/data' do
  content_type 'text/plain'
  `ls -l data/*`
end

get '/ls/runs' do
  content_type 'text/plain'
  `ls -l runs/*`
end

get '/git/log' do
  content_type 'text/plain'
  `git log`
end

get '/ps' do
  content_type 'text/plain'
  `ps ux`
end

get '/pr/code' do
  content_type 'text/plain'
  `pr -n -t parse.leg.c`
end
