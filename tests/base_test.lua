require "dokidoki.module" [[]]

base = require "dokidoki.base"

function all_equal (a, b)
  if a == b then
    return true
  elseif a == nil or b == nil then
    return false
  else
    for k, v in pairs(a) do
      if v ~= b[k] then
        return false
      end
    end
    for k, v in pairs(b) do
      if v ~= a[k] then
        return false
      end
    end
    return true
  end
end

function square (x)
  return x * x;
end

function add1 (x)
  return x + 1;
end

-- first test my equality function :3
assert(all_equal({a = 1, b = 2}, {b = 2, a = 1}))
assert(not all_equal({a = 1, b = 2}, {b = 1, a = 2}))
assert(not all_equal({a = 1, b = 2}, {a = 1}))
assert(not all_equal({a = 1}, {a = 1, b = 2}))
assert(not all_equal({a = 1}, {}))
assert(not all_equal({}, {a = 1}))

assert(all_equal(base.map(square, {4, 2, 3}), {16, 4, 9}))
assert(all_equal(base.map(add1, {x = 1, y = 4}), {x = 2, y = 5}))
assert(all_equal(base.array_filter(base.identity, {1, true, false, "hello"}),
                 {1, true, "hello"}))
assert(all_equal(base.build_array(3, square), {1, 4, 9}))
assert(all_equal(base.build_array(5, base.identity), {1, 2, 3, 4, 5}))
assert(all_equal({base.identity(1, "a", true)}, {1, "a", true}))
