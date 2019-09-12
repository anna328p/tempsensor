require 'sinatra'
require 'json'

post '/' do
  puts JSON.parse(request.body.string)['temperature']
end
