import { inflate } from 'pako';

function smDecode(input) {
  return JSON.parse(inflate(
    window.atob(input),
    { to: 'string' },
  ));
}

const maxL = [15, 9, 22, 12, 10, 3, 4];

export function smPrepare(input, reversed) {
  /* eslint-disable comma-style */
  const [
    , // Personal information, discarded
    ...pref // SM information
  ] = smDecode(input);
  /* eslint-enable comma-style */
  pref.length = 7;
  pref[2][4] = null; // C3Q4 must by omitted
  pref[2][7] = null; // C3Q7 must by omitted

  const res = [];
  [...pref].forEach((c, cid) => {
    c.length = maxL[cid];
    [...c].forEach((q) => {
      if (q === undefined || q === null) {
        res.push(0, 0);
      } else {
        if (reversed) {
          q.reverse();
        }
        [...q].forEach((o) => {
          if (o === undefined || o === null) {
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

export function smDiscuss(result) {
  let res = '';
  const rst = [...result];
  maxL.forEach((ml, ci) => {
    const c = rst.splice(0, ml * 2);
    const asbm = !!c[0];
    const ambs = !!c[1];
    for (let qi = 0; qi < ml; qi += 1) {
      if (asbm && c[2 * qi]) {
        res += `AliceS+BobM\tC${1 + ci}Q${qi}\t${c[2 * qi]}\n`;
      }
      if (ambs && c[2 * qi + 1]) {
        res += `AliceM+BobS\tC${1 + ci}Q${qi}\t${c[2 * qi + 1]}\n`;
      }
    }
  });
  return res;
}
