if (exc.fromMakeError) {
      result = exc;
    } else {
      result = {
        fromMakeError: true,
        name: exc.name || "ERROR",
        message: String(exc),
        stack: exc.stack,
      };
      for (const attr in exc) {
        result[attr] = exc[attr];
      }
    }
    if (info) {
      for (const attr of Object.keys(info)) {
        result[attr] = info[attr];
      }
    }
    return result;
  }

  /** Wrap the function, and if it raises any exceptions then call unhandled() */
  exports.watchFunction = function watchFunction(func, quiet) {
    return function() {
      try {
        return func.apply(this, arguments);
      } catch (e) {
        if (!quiet) {
          exports.unhandled(e);
        }
        throw e;
      }
    };
  };

  exports.watchPromise = function watchPromise(promise, quiet) {
    return promise.catch((e) => {
      if (quiet) {
        if (!e.noReport) {
          log.debug("------Error in promise:", e);
          log.debug(e.stack);
        }
      } else {
        if (!e.noReport) {
          log.error("------Error in promise:", e);
          log.error(e.stack);
        }
        exports.unhandled(makeError(e));
      }
      throw e;
    });
  };

  exports.registerHandler = function(h) {
    if (handler) {
      log.error("registerHandler called after handler was already registered");
      return;
    }
    handler = h;
    for (const error of queue) {
      handler(error);
    }
    queue = [];
  };

  return exports;
})();
null;
PK