import { inflate } from 'pako';

function smDecode(input) {
  return JSON.parse(inflate(
    window.atob(input),
    { to: 'string' },
  ));
}

export function smPrepare(input) {
  /* eslint-disable comma-style */
  const [
    , // Personal information, discarded
    ...pref // SM information
  ] = smDecode(input);
  /* eslint-enable comma-style */
  pref[2].splice(7, 1); // C3Q7 must by omitted

  const res = [];
  pref.forEach((c) => {
    c.forEach((x) => {
      if (x === null) {
        res.push(0, 0);
      } else {
        x.forEach((o) => {
          if (o === null) {
            res.push(0);
          } else {
            res.push(3 - o);
          }
        });
      }
    });
  });
  return res;
}
