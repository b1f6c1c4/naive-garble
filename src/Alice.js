import React from 'react';
import { smPrepare } from './sm';

const { Module } = window;

export default class Alice extends React.PureComponent {
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
        let output = '';
        res.forEach((v) => {
          const o = new Module.Alice4(v);
          output += o.garble();
          os.push(o);
        });
        this.setState({
          obj: os,
          type: 1,
          input: '',
          output,
        });
        break;
      }
      case 1:
      case 3:
        this.setState({
          type: type + 1,
          input: '',
          output: '',
        });
        break;
      case 2: {
        let str = input;
        let output = '';
        obj.forEach((o) => {
          output += o.receive(str.substr(0, Module.inquirySize4 * 2));
          str = str.substr(Module.inquirySize4 * 2);
        });
        this.setState({
          type: 3,
          input: '',
          output,
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
            <p>Send the following message to Bob.</p>
            <pre>{output}</pre>
            <input type="submit" value="Next" />
          </form>
        );
      case 2:
        return (
          <form onSubmit={this.handleSubmit}>
            <p>Get a message from Bob and paste it here.</p>
            <input type="text" value={input} onChange={this.handleChange} />
            <input type="submit" value="Next" />
          </form>
        );
      default:
        return (
          <form>
            <p>You&apos;re all set. Bob&apos;ll tell you the result.</p>
          </form>
        );
    }
  }
}
