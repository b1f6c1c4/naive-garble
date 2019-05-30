import React from 'react';
import { smPrepare } from './sm';
import download from './download';

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
    const { input } = this.state;
    this.progress(input);
  };

  progress = (input) => {
    const { kink } = this.props;
    const { obj, type } = this.state;
    switch (type) {
      case 0: {
        const res = kink ? smPrepare(input) : [parseInt(input, 10)];
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

  handleUpload = ({ target: { files: [f] } }) => {
    if (!f) {
      return;
    }
    const fr = new FileReader();
    fr.onload = ({ target: { result } }) => {
      this.progress(result);
    };
    fr.readAsText(f);
  };

  handleDownload = () => {
    const { type, output } = this.state;
    download(`${type}-alice-to-bob`, output);
  };

  render() {
    const { kink } = this.props;
    const { type, input, output } = this.state;

    const prompt = kink ? (
      <h2>What&apos;s your base64 string?</h2>
    ) : (
      <h2>What&apos;s your number? (0, 1, 2, or 3 only)</h2>
    );

    const warn = kink ? (
      <p className="warn">Warning: it may take 10 seconds to 3 minutes to complete the computation. PLEASE BE PATIENT.</p>
    ) : undefined;

    switch (type) {
      case 0:
        return (
          <form onSubmit={this.handleSubmit}>
            {prompt}
            <input type="text" value={input} onChange={this.handleChange} />
            <input type="submit" value="Next" />
            {warn}
          </form>
        );
      case 1:
      case 3:
        return (
          <form onSubmit={this.handleSubmit}>
            <h2>Send the following message to Bob.</h2>
            <input type="button" value="Download" onClick={this.handleDownload} />
            <pre>{output}</pre>
            <input type="submit" value="Next" />
          </form>
        );
      case 2:
        return (
          <form onSubmit={this.handleSubmit}>
            <h2>Get a message from Bob and paste it here.</h2>
            <input type="text" value={input} onChange={this.handleChange} />
            <input type="submit" value="Next" />
            <p>--Or--</p>
            <input type="file" onChange={this.handleUpload} />
            {warn}
          </form>
        );
      default:
        return (
          <form>
            <h2>You&apos;re all set.</h2>
            <p>Bob&apos;ll tell you the result.</p>
          </form>
        );
    }
  }
}
