import React from 'react';
import { smPrepare } from './sm';

const { Module } = window;

export default class Bob extends React.PureComponent {
  state = {
    obj: null,
    type: 0,
    input: '',
    output: '',
  };

  componentWillUnmount() {
    const { obj } = this.state;
    obj.forEach((o) => o.remove());
  }

  handleChange = ({ target }) => {
    this.setState({ input: target.value });
  };

  handleSubmit = (e) => {
    e.preventDefault();
    const { kink } = this.props;
    const { obj, type, input } = this.state;
    switch (type) {
      case 0: {
        const res = kink ? smPrepare(input) : res = [parseInt(input, 10)];
        const os = [];
        res.forEach((v) => {
          os.push(new Module.Bob4(v));
        });
        this.setState({
          obj: os,
          type: 1,
          input: '',
          output: '',
        });
        break;
      }
      case 1: {
        let str = input;
        let output = '';
        obj.forEach((o) => {
          output += o.inquiry(str.substr(0, Module.garbleSize4 * 2));
          str = str.substr(Module.garbleSize4 * 2);
        });
        this.setState({
          type: 2,
          input: '',
          output,
        });
        break;
      }
      case 2:
        this.setState({
          type: 3,
          input: '',
          output: '',
        });
        break;
      case 3: {
        let str = input;
        const output = [];
        obj.forEach((o) => {
          let r = o.evaluate(str.substr(0, Module.receiveSize4 * 2));
          if (r === -1)
            r = NaN;
          output.push(r);
          str = str.substr(Module.receiveSize4 * 2);
        });
        this.setState({
          type: 4,
          input: '',
          output: ''+output,
        });
        break;
      }
      case 3: {
        let o = obj.evaluate(input);
        if (o === -1) {
          o = 'ERROR';
        }
        this.setState({
          type: 4,
          input: '',
          output: o,
        });
        break;
      }
      default:
        break;
    }
  };

  render() {
    const { kink } = this.props;
    const { type, input, output } = this.state;

    const prompt = kink ? (
      <p>What&apos;s your base64 string?</p>
    ) : (
      <p>What&apos;s your number? (0, 1, 2, or 3 only)</p>
    );

    switch (type) {
      case 0:
        return (
          <form onSubmit={this.handleSubmit}>
            {prompt}
            <input type="text" value={input} onChange={this.handleChange} />
            <input type="submit" value="Next" />
          </form>
        );
      case 1:
      case 3:
        return (
          <form onSubmit={this.handleSubmit}>
            <p>Get a message from Alice and paste it here.</p>
            <input type="text" value={input} onChange={this.handleChange} />
            <input type="submit" value="Next" />
          </form>
        );
      case 2:
        return (
          <form onSubmit={this.handleSubmit}>
            <p>Send the following message to Alice.</p>
            <pre>{output}</pre>
            <input type="submit" value="Next" />
          </form>
        );
      default:
        return (
          <form>
            <p>The minimum of your number and Alice&apos;s number is:</p>
            <pre>{output}</pre>
          </form>
        );
    }
  }
}
