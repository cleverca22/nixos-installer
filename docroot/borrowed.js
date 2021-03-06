// taken from https://github.com/NixOS/nixos-homepage/blob/master/nixos/options.tt

var optionData = [];

var results = [];

var resultsPerPage = 15;

var curPage = 0;
var lastPage = 0;

function updateTable() {
  if (0 > curPage) curPage = 0;
  if (curPage > lastPage) curPage = lastPage;

  var body = $('#search-results tbody');
  $('tr', body).remove();

  $('.back').toggleClass('disabled', curPage == 0);
  $('.forward').toggleClass('disabled', curPage >= lastPage);

  var start = curPage * resultsPerPage;
  var end = start + resultsPerPage;
  if (end > results.length) end = results.length;
  res = results.slice(start, end);

  if (results.length == 0) {
    $('#results-wrapper').hide();
    $('#how-many').text('No matching options were found.');
    return;
  }

  $('#how-many').text(
    'Showing results ' + (start + 1) + '-' + end + ' of ' + results.length + '.');

  var odd = true;
  res.forEach(function(optName) {
    var opt = optionData[optName];
    body.append($('<tr/>', { optName: optName })
      .addClass('result')
      .addClass(odd ? 'odd' : 'even')
      .click(showOption)
      .append($('<td/>', { class: 'option', text: optName }))
    );
    odd = !odd;
  });

  $('#results-wrapper').show();
};

function refilter() {
  var options = Object.keys(optionData);

  results = options;

  var words = $('#search').val().toLowerCase().split(/ +/).filter(Boolean);

  if (words.length > 0) {
    results = results.filter(function (optName) {
      var opt = optionData[optName];
      function match(word) {
        return (optName.toLowerCase().indexOf(word) != -1
          || (opt.description || '').toLowerCase().indexOf(word) != -1);
      };

      return words.every(match);
    });
  }

  curPage = 0;
  lastPage = (results.length - 1) / resultsPerPage >> 0;

  updateTable();
};

// This function is used for pretty printing Nix values which have been
// exported into JSON, and read as JavaScript objects.  Nix is exported to
// JSON, but JavaScript has terrible serialisation of its internal values.
function ppNix(indent, v) {
  var len = 0;
  var outerIndent = indent;
  indent += "  ";

  function ppRec(v) {
    return ppNix(indent, v)
  }
  function ppIndent(i, firstLine) {
    return i ? indent : firstLine;
  }
  function ppNL(i) {
    return len <= i ? ' ' : '\n';
  }
  function needIndentation(v) {
    if (typeof v == "object") {
      if (Array.isArray(v)) {
        if (v.some(needIndentation))
          return true;
        return false;
      }
      if (Object.keys(v).some(propNeedIndentation(v)))
        return true;
      return false;
    }
    if (typeof v == "string")
      return v.indexOf('"') != -1 || v.indexOf('\n') != -1;
    return false;
  }
  function propNeedIndentation(obj) {
    return function (key) {
      return needIndentation(obj[key]);
    }
  }

  if (v == null) return "null";

  // JavaScript consider both objects and arrays as objects.
  if (typeof v == "object") {

    if (Array.isArray(v)) {
      len = v.length;
      // If none of the element inside it need to have indentation levels, then
      // we can just print the whole array on one line.
      if (!v.some(needIndentation)) {
        if (len == 0)
          return '[]';
        var res = '[ ';
        for (var i = 0; i < v.length; i++)
          res += ppNix(indent, v[i]) + ' ';
        return res + ']';
      }

      // Print an array on multiple lines as it contains some complex elements.
      var res = '[';
      for (var i = 0; i < v.length; i++)
        res += ppIndent(i, ' ') + ppNix(indent, v[i]) + ppNL(1);
      return res + ']';
    }

    // Some example attribute are using the literalExample function to wrap
    // their content as it cannot be serialize properly. So if we detect such
    // example, escape characters and return the text without pretty-printing.
    if (v._type == "literalExample") {
      v = ((v.text || '') + '').replace('<', '&lt;').replace('>', '&gt;');
      return v;
    }

    // Print an attribute set.  Always indent the first line
    var attrset = Object.keys(v);
    len = attrset.length;
    if (!attrset.some(propNeedIndentation(v))) {
      var res = '{ ';
      for (var i = 0; i < len; i++) {
        var attrName = attrset[i];
        var value = v[attrName];
        res += attrName + ' = ';
        res += ppNix(indent, value);
        res += '; ';
      }
      return res + '}'
    }

    var res = '{\n';
    for (var i = 0; i < len; i++) {
      var attrName = attrset[i];
      var value = v[attrName];
      var ni = needIndentation(value);
      res += indent;
      res += attrName + ' =' + (ni ? '\n' + indent + '  ' : ' ')
      res += ppNix(indent + '  ', value);
      res += ';\n';
    }
    return res + outerIndent + '}';
  }

  if (typeof v == "string") {
    v = v.replace('<', '&lt;').replace('>', '&gt;');
    if (v.indexOf('"') == -1 && v.indexOf('\n') == -1) {
      if (/^pkgs\./.test(v))
        return '' + v;
      return '"' + v + '"';
    }
    var lines = v.split('\n');
    var res = "''\n";
    for (var i = 0; i < lines.length; i++)
      res += indent + lines[i] + '\n';
    return res + outerIndent + "''";
  }

  return '' + v;
}

function showOption() {
  var optName = $(this).attr('optName');
  var opt = optionData[optName];

  var expanded = $(this).attr('expanded');
  $('tr.details', $(this).parent()).remove();
  $('tr', $(this).parent()).removeAttr('expanded');
  if (expanded) return;

  var details = $('#details-template').clone().removeAttr('id').show();

  var x = $.parseXML("<div>" + opt.description + "</div>");
  $('.description', details).empty().append($(x).text());

  if ('default' in opt)
    $('.default', details).empty().addClass('pre').text(ppNix('', opt.default));

  if ('example' in opt)
    $('.example', details).empty().addClass('pre').text(ppNix('', opt.example));

  if ('type' in opt)
    $('.type', details).empty().addClass('pre').text(opt.type);

  $(".use", details).click(function () {
    var config = $("#configuration");
    var newval = config.val() + "\n" + optName+" = "+ppNix('', opt.default)+";";
    config.val(newval);
  });

  if (opt.declarations.length > 0) {
    var res = $('.declared-in', details);
    res.empty();
    var first = true;
    opt.declarations.forEach(function(module) {
      var url = "https://github.com/NixOS/nixpkgs/tree/master/" + module;
      if (!first) res.append(', '); first = false;
      res.append($('<a/>', { href: url }).append($('<tt/>').text(module)));
    });
  }

  $(this).after($('<tr/>')
    .addClass('details')
    .append($('<td/>', { colspan: 3 })
      .append(details))
  ).attr('expanded', 1);
};
