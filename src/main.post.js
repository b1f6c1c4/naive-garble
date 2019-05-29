Module.onRuntimeInitialized = function() {

  function toHex(b) {
    var str = '';
    for (var i = 0; i < b.length; i++) {
      var s = b[i].toString(16);
      if (s.length < 2)
        str += '0' + s;
      else
        str += s;
    }
    return str;
  }

  function fromHex(str, b) {
    if (str.length != b.length * 2)
      throw new Error('Incorrect length');
    for (var i = 0; i < b.length; i++) {
      b[i] = parseInt(str.substr(i * 2, 2), 16);
    }
  }

  function withBuffer(sz, cb) {
    var ptr = Module._malloc(sz);
    try {
      var buff = new Uint8Array(Module.HEAPU8.buffer, ptr, sz);
      return cb(buff);
    } finally {
      Module._free(ptr);
    }
  }

  function make(m) {
    // template <size_t M>
    // size_t garble_size<M>()
    var garble_size = Module.ccall(`_Z11garble_sizeILm${m}EEmv`, 'number', []);
    // template <size_t M>
    // size_t inquiry_size<M>()
    var inquiry_size = Module.ccall(`_Z12inquiry_sizeILm${m}EEmv`, 'number', []);
    // template <size_t M>
    // size_t receive_size<M>()
    var receive_size = Module.ccall(`_Z12receive_sizeILm${m}EEmv`, 'number', []);

    function Alice(a) {
      // template <size_t M>
      // auto alice_t<M>::create(size_t)
      this.self = Module.ccall(`_ZN7alice_tILm${m}EE6createEm`, 'number', ['number'], a);
    }

    Alice.prototype.garble = function() {
      var self = this.self;
      return withBuffer(garble_size, function(buffOut) {
        // template <size_t M>
        // void alice_t<M>::garble(char *)
        Module.ccall(`_ZN7alice_tILm${m}EE6garbleEPc`, 'null', ['number', 'number'], [self, buffOut.byteOffset]);
        return toHex(buffOut);
      });
    };

    Alice.prototype.receive = function(input) {
      var self = this.self;
      return withBuffer(inquiry_size, function(buffIn) {
        fromHex(input, buffIn);
        return withBuffer(receive_size, function(buffOut) {
          // template <size_t M>
          // void alice_t<M>::receive(var char *, char *)
          Module.ccall(`_ZN7alice_tILm${m}EE7receiveEPKcPc`, 'null', ['number', 'number', 'number'], [self, buffIn.byteOffset, buffOut.byteOffset]);
          return toHex(buffOut);
        });
      });
    };

    Alice.prototype.remove = function() {
      // template <size_t M>
      // void alice_t<M>::remove(alice_t *)
      Module.ccall(`_ZN7alice_tILm${m}EE6removeEPS0_`, 'void', ['number'], this.self);
    }

    function Bob(a) {
      // template <size_t M>
      // bob_t<M>::bob_t(size_t)
      this.self = Module.ccall(`_ZN5bob_tILm${m}EE6createEm`, 'number', ['number'], a);
    }

    Bob.prototype.inquiry = function(input) {
      var self = this.self;
      return withBuffer(garble_size, function(buffIn) {
        fromHex(input, buffIn);
        return withBuffer(inquiry_size, function(buffOut) {
          // template <size_t M>
          // void bob_t<M>::inquiry(var char *, char *)
          Module.ccall(`_ZN5bob_tILm${m}EE7inquiryEPKcPc`, 'null', ['number', 'number', 'number'], [self, buffIn.byteOffset, buffOut.byteOffset]);
          return toHex(buffOut);
        });
      });
    };

    Bob.prototype.evaluate = function(input) {
      var self = this.self;
      return withBuffer(receive_size, function(buffIn) {
        fromHex(input, buffIn);
        // template <size_t M>
        // size_t bob_t<M>::evaluate(var char *)
        return Module.ccall(`_ZN5bob_tILm${m}EE8evaluateEPKc`, 'number', ['number', 'number'], [self, buffIn.byteOffset]);
      });
    };

    Bob.prototype.remove = function() {
      // template <size_t M>
      // void bob_t<M>::remove(bob_t *)
      Module.ccall(`_ZN5bob_tILm${m}EE6removeEPS0_`, 'void', ['number'], this.self);
    }

    var obj = {};
    obj['Alice' + m] = Alice;
    obj['Bob' + m] = Bob;
    return obj;
  }

  Object.assign(this, make(2));
  Object.assign(this, make(4));

};
